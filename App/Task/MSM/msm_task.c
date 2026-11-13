#include "msm_task.h"
#include "global_def.h"
#include "module_msm.h"
#include "msm_port.h"
#include "app_commands.h"
#include "app_freertos.h"

/* Set to 0 to disable MSM diagnostics without changing state-machine logic. */
#ifndef MSM_DEBUG_ENABLE
#define MSM_DEBUG_ENABLE 1
#endif

void MsmTask_Run(void *argument)
{
    (void)argument;
    ModuleMsm_t msm;
    static AppTaskRuntime_t ctx;
    bool publish_pending=true;
    uint32_t broadcast_pending=0U;
    uint32_t sequence=0U;
    static ControlResultMsg_t reply;
    bool request_active=false, reply_ready=false;
    const MsmHooks_t hooks = {
        .enter = MsmPort_Enter,
        .exit = MsmPort_Exit
    };
    /* 10 ms polling scaffold; hardware protection is independent. */
    const uint32_t period = osKernelGetTickFreq() / 100U +
                           ((osKernelGetTickFreq() % 100U) != 0U);

    ModuleMsm_Init(&msm, &hooks, NULL);
#if MSM_DEBUG_ENABLE
    uint32_t last_status_tick = osKernelGetTickCount();
    const uint32_t status_interval = osKernelGetTickFreq() * 5U;
    debug_printf("[MSM] start state=%s gen=%" PRIu32 " poll=%" PRIu32 " ticks\r\n",
                 ModuleMsm_StateName(msm.state), msm.generation, period);
    debug_printf("[MSM] hardware hooks pending; waiting for real completion\r\n");
#endif
    for (;;) {
        if (!request_active && broadcast_pending==0U) {
            MsmMsg_t command;
            if (osMessageQueueGet(MsmMsgQueueHandle,&command,NULL,0U)==osOK) {
                request_active=true;
                reply=(ControlResultMsg_t){.header=command.header,
                    .status=command.header.opcode==APP_CMD_MSM_PUBLISH ? APP_RESULT_OK : APP_RESULT_UNSUPPORTED};
                reply.header.source=APP_TASK_MSM;
                if (reply.status==APP_RESULT_OK) { publish_pending=true; }
                else { reply_ready=true; }
            }
        }
        MsmInputs_t inputs;
        MsmPort_ReadInputs(&msm, &inputs);
        const bool have_snapshot=AppTask_ReadSnapshot(&ctx);
        if (have_snapshot && ctx.snapshot.fault.valid) {
            inputs.fault |= ctx.snapshot.fault.latched_bits != 0U;
        }
#if MSM_DEBUG_ENABLE
        const MsmState_t previous_state = msm.state;
#endif
        const bool changed = ModuleMsm_Step(&msm, &inputs);
#if MSM_DEBUG_ENABLE
        if (changed) {
            debug_printf("[MSM] %s -> %s gen=%" PRIu32 "\r\n",
                         ModuleMsm_StateName(previous_state),
                         ModuleMsm_StateName(msm.state), msm.generation);
        }
        const uint32_t now = osKernelGetTickCount();
        if ((uint32_t)(now - last_status_tick) >= status_interval) {
            last_status_tick = now;
            debug_printf("[MSM] state=%s gen=%" PRIu32 "\r\n",
                         ModuleMsm_StateName(msm.state), msm.generation);
        }
#endif
        publish_pending |= changed;
        if (publish_pending) {
            const ModuleStatus_t module = {
                .state=(uint32_t)msm.state, .power=SHARED_POWER_UNKNOWN,
                .sample_tick=osKernelGetTickCount(), .valid=true,
                .broadcast_sequence=sequence+1U
            };
            if (SharedData_UpdateModule(&module)) {
                sequence=module.broadcast_sequence;
                publish_pending=false;
                if (request_active && !reply_ready) {
                    reply.detail=(int32_t)sequence;
                    reply_ready=true;
                }
                broadcast_pending |= APP_OTHER_TASKS(APP_TASK_MSM);
            } else { ++ctx.status.failures; }
        }
        if (broadcast_pending != 0U) {
            broadcast_pending=AppEvents_NotifyTargets(broadcast_pending, APP_EVT_MODULE);
            if (broadcast_pending != 0U) { ++ctx.status.failures; }
        }
        if (request_active && reply_ready && broadcast_pending==0U && AppCommands_SendResult(&reply)) {
            request_active=false;
            reply_ready=false;
        }
        AppTask_PublishStatus(&ctx, APP_TASK_MSM);
        /* Event bits wake reevaluation; hardware state inputs remain TODO.
         * Retain timed wakeups for pending operations and diagnostics. */
        AppTask_WaitEvents(&ctx, MSM_EVT_ALL, 10U);
    }
}
