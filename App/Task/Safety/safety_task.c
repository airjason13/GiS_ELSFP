#include "safety_task.h"
#include "global_def.h"

static uint32_t Safety_ProtectAndGetFaults(const SharedSnapshot_t *snapshot)
{
    (void)snapshot;
    /* TODO: evaluate interlocks and real protection limits; execute required
     * hardware shutdown BEFORE returning newly detected fault bits.
     * No hardware protection is implemented by this placeholder. */
    return 0U;
}

void SafetyTask_Run(void *argument)
{
    (void)argument;
    static AppTaskRuntime_t ctx;
    uint32_t pending_faults=0U, pending_targets=0U;
    for (;;) {
        if (AppTask_ReadSnapshot(&ctx)) {
            pending_faults |= Safety_ProtectAndGetFaults(&ctx.snapshot) &
                              ~ctx.snapshot.fault.latched_bits;
            /* TODO: real protection must execute BEFORE sharing/notification.
             * TODO: populate pending_faults from actual protection checks. */
        }
        if (pending_faults != 0U) {
            if (SharedData_LatchFault(pending_faults, osKernelGetTickCount())) {
                pending_faults=0U;
                pending_targets |= APP_TASK_BIT(APP_TASK_HOST) | APP_TASK_BIT(APP_TASK_MSM);
            } else { ++ctx.status.failures; }
        }
        if (pending_targets != 0U) {
            pending_targets=AppEvents_NotifyTargets(pending_targets, HOST_EVT_FAULT);
            if (pending_targets != 0U) { ++ctx.status.failures; }
        }
        AppTask_PublishStatus(&ctx, APP_TASK_SAFETY);
        AppTask_WaitEvents(&ctx, APP_EVT_MODULE, 10U);
    }
}
