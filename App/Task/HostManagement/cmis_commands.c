#include "cmis_commands.h"
#include "global_def.h"
#include "app_commands.h"
#include "app_freertos.h"

void CmisCommands_ProcessQueues(void)
{
    static HostCmdMsg_t command;
    static ControlResultMsg_t completion;
    static bool active, publish;
    static uint32_t started;
    static uint16_t worker;
    ControlResultMsg_t result;
    /* Only Host consumes ControlResultMsgQueue. Reject late/unrelated replies. */
    for (unsigned i=0; i<8U && osMessageQueueGet(ControlResultMsgHandle,&result,NULL,0U)==osOK; ++i) {
        if (active && !publish && result.header.request_id==command.header.request_id &&
            result.header.opcode==command.header.opcode && result.header.source==worker) {
            completion=result;
            publish=true;
        } else {
            debug_printf("[HOST] ignored late/unmatched result id=%" PRIu32 "\r\n",result.header.request_id);
        }
    }
    if (active && !publish && (uint32_t)(osKernelGetTickCount()-started)>=2U*osKernelGetTickFreq()) {
        completion.status=APP_RESULT_TIMEOUT;
        publish=true; /* Timeout is not cancellation of the worker operation. */
    }
    if (publish) {
        if (SharedData_UpdateCommandResult((AppTaskId_t)command.header.source,&completion)) {
            debug_printf("[HOST] result id=%" PRIu32 " op=0x%04X status=%" PRId32 "\r\n",
                         completion.header.request_id,(unsigned)completion.header.opcode,completion.status);
            publish=false;
            active=false;
        }
        return;
    }
    if (active || osMessageQueueGet(HostCmdMsgQueueHandle,&command,NULL,0U)!=osOK) { return; }
    if (command.header.source>=APP_TASK_COUNT) {
        debug_printf("[HOST] invalid command source\r\n");
        return;
    }
    active=true;
    started=osKernelGetTickCount();
    worker=APP_TASK_HOST;
    completion=(ControlResultMsg_t){.header=command.header,.status=APP_RESULT_UNSUPPORTED};
    bool accepted=false;
    if (command.length!=0U) {
        completion.status=APP_RESULT_INVALID;
    } else if (command.header.opcode==APP_CMD_LASER_PING) {
        worker=APP_TASK_LASER;
        const LaserMsg_t laser={.header=command.header};
        accepted=AppCommands_SendLaser(&laser);
        completion.status=APP_RESULT_BUSY;
    } else if (command.header.opcode==APP_CMD_MSM_PUBLISH) {
        worker=APP_TASK_MSM;
        const MsmMsg_t msm={.header=command.header};
        accepted=AppCommands_SendMsm(&msm);
        completion.status=APP_RESULT_BUSY;
    }
    completion.header.source=worker;
    publish=!accepted;
    debug_printf("[HOST] dispatch id=%" PRIu32 " op=0x%04X queued=%u\r\n",
                 command.header.request_id,(unsigned)command.header.opcode,(unsigned)accepted);
}
