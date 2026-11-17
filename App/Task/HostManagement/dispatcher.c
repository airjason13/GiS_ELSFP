#include "dispatcher.h"
#include "global_def.h"
#include "app_msg.h"
#include "app_freertos.h"
#include <string.h>

/* Return true only when the worker queue accepted the command.
 * On false, error is the immediate result published by Host. */
typedef bool (*CommandDispatch_t)(const HostCmdMsg_t *command, int32_t *error);
typedef struct {
    AppOpcode_t opcode;
    AppTaskId_t worker;
    CommandDispatch_t dispatch;
} CommandRoute_t;

static bool DecodeTarget(const HostCmdMsg_t *command, int32_t maximum,
                         int32_t *target, int32_t *error)
{
    if (command->length!=sizeof(*target)) {
        *error=APP_RESULT_INVALID;
        return false;
    }
    /* Internal native int32 payload, not a CMIS wire encoding. */
    memcpy(target,command->data,sizeof(*target));
    if (*target<0 || *target>maximum) {
        *error=APP_RESULT_INVALID;
        return false;
    }
    return true;
}

static bool SendLaserSetting(const HostCmdMsg_t *command, int32_t maximum,
                                 int32_t *error)
{
    LaserMsg_t message={.header=command->header};
    if (!DecodeTarget(command,maximum,&message.target,error)) { return false; }
    *error=APP_RESULT_BUSY;
    return AppMsg_SendLaser(&message);
}

static bool SendLaserEnable(const HostCmdMsg_t *command, int32_t *error)
{ return SendLaserSetting(command,1,error); }
static bool SendLaserBias(const HostCmdMsg_t *command, int32_t *error)
{ return SendLaserSetting(command,INT32_MAX,error); }
static bool SendLaserPower(const HostCmdMsg_t *command, int32_t *error)
{ return SendLaserSetting(command,INT32_MAX,error); }
static bool SendLaserApc(const HostCmdMsg_t *command, int32_t *error)
{ return SendLaserSetting(command,1,error); }

static bool SendMsmPublish(const HostCmdMsg_t *command, int32_t *error)
{
    if (command->length!=0U) { *error=APP_RESULT_INVALID; return false; }
    const MsmMsg_t message={.header=command->header};
    *error=APP_RESULT_BUSY;
    return AppMsg_SendMsm(&message);
}

/* Add a handler and one row here for each new command.
 * Explicit matching avoids depending on contiguous opcode numbers. */
static const CommandRoute_t routes[] = {
    /* ==================== Laser ==================== */
    { APP_CMD_LASER_ENABLE, APP_TASK_LASER, SendLaserEnable },
    { APP_CMD_LASER_BIAS,   APP_TASK_LASER, SendLaserBias },
    { APP_CMD_LASER_POWER,  APP_TASK_LASER, SendLaserPower },
    { APP_CMD_LASER_APC,    APP_TASK_LASER, SendLaserApc },
    /* ===================== MSM ===================== */
    { APP_CMD_MSM_PUBLISH,  APP_TASK_MSM,   SendMsmPublish }
};

static bool Host_SendRequestToWorker(const HostCmdMsg_t *command, uint16_t *worker,
                            int32_t *error)
{
    *worker=APP_TASK_HOST;
    for (unsigned i=0U; i<sizeof(routes)/sizeof(routes[0]); ++i) {
        if (command->header.opcode==routes[i].opcode) {
            *worker=(uint16_t)routes[i].worker;
            return routes[i].dispatch(command,error);
        }
    }
    *error=command->length!=0U ? APP_RESULT_INVALID : APP_RESULT_UNSUPPORTED;
    return false;
}

#define COMMAND_RESULT_TIMEOUT_SECONDS 2U
#define WORKER_RESULTS_PER_POLL 8U

typedef struct {
    HostCmdMsg_t request;
    ControlResultMsg_t result;
    uint32_t result_wait_started_tick;
    uint16_t worker;
    bool waiting_for_result;
    bool result_ready;
} HostCommandContext_t;

/* Host Task owns this context; one request at a time. */
static HostCommandContext_t ctx;

/* Worker Tasks -> Host. */
static void Host_ReceiveWorkerResults(void)
{
    ControlResultMsg_t result;
    for (unsigned i=0U; i<WORKER_RESULTS_PER_POLL &&
         osMessageQueueGet(WorkerResultQueueHandle,&result,NULL,0U)==osOK; ++i) {
        if (ctx.waiting_for_result && !ctx.result_ready &&
            result.header.request_id==ctx.request.header.request_id &&
            result.header.opcode==ctx.request.header.opcode &&
            result.header.source==ctx.worker) {
            ctx.result=result;
            ctx.result_ready=true;
        } else {
            debug_printf("[HOST] ignored late/unmatched result id=0x%08" PRIX32 "\r\n",result.header.request_id);
        }
    }
}

static void Host_CheckResultTimeout(uint32_t now_tick)
{
    const uint32_t result_timeout_ticks=COMMAND_RESULT_TIMEOUT_SECONDS*osKernelGetTickFreq();
    if (ctx.waiting_for_result && !ctx.result_ready &&
        (uint32_t)(now_tick-ctx.result_wait_started_tick)>=result_timeout_ticks) {
        ctx.result.status=APP_RESULT_TIMEOUT;
        ctx.result_ready=true; /* Timeout does not cancel worker execution. */
    }
}

/* Host -> requester's shared result mailbox. Retry if publication fails. */
static void Host_ReplyToRequester(void)
{
    if (SharedData_UpdateCommandResult((AppTaskId_t)ctx.request.header.source,&ctx.result)) {
        debug_printf("[HOST] result id=0x%08" PRIX32 " op=0x%04X status=%" PRId32 "\r\n",
                     ctx.result.header.request_id,(unsigned)ctx.result.header.opcode,ctx.result.status);
        ctx.result_ready=false;
        ctx.waiting_for_result=false;
    }
}

/* Requesters -> Host -> Worker Tasks. */
static void Host_ReceiveAndDispatchRequest(uint32_t now_tick)
{
    if (osMessageQueueGet(HostRequestQueueHandle,&ctx.request,NULL,0U)!=osOK) { return; }
    if (ctx.request.header.source>=APP_TASK_COUNT) {
        debug_printf("[HOST] invalid request source\r\n");
        return;
    }
    ctx.waiting_for_result=true;
    ctx.result_wait_started_tick=now_tick;
    ctx.result=(ControlResultMsg_t){.header=ctx.request.header,.status=APP_RESULT_UNSUPPORTED};
    const bool accepted=Host_SendRequestToWorker(&ctx.request,&ctx.worker,&ctx.result.status);
    ctx.result.header.source=ctx.worker;
    ctx.result_ready=!accepted;
    debug_printf("[HOST] dispatch id=0x%08" PRIX32 " op=0x%04X queued=%u\r\n",
                 ctx.request.header.request_id,(unsigned)ctx.request.header.opcode,(unsigned)accepted);
}

void Dispatcher_Process(void)
{
    const uint32_t now_tick=osKernelGetTickCount();
    Host_ReceiveWorkerResults();
    Host_CheckResultTimeout(now_tick);
    if (ctx.result_ready) {
        Host_ReplyToRequester();
        return;
    }
    if (ctx.waiting_for_result) { return; }
    Host_ReceiveAndDispatchRequest(now_tick);
}
