#include "supervision_task.h"
#include "global_def.h"

static void Ddm_Evaluate(const SharedSnapshot_t *snapshot, AlarmStatus_t *alarm)
{
    (void)snapshot;
    /* TODO: configure actual alarm/warning limits and hysteresis; fill bits
     * and set valid only after the required input data can be evaluated. */
    alarm->valid = false;
}

void SupervisionTask_Run(void *argument)
{
    (void)argument;
    static AppTaskRuntime_t ctx;
    AlarmStatus_t last = {0};
    bool published = false, notify_pending = false;
    for (;;) {
        if (AppTask_ReadSnapshot(&ctx)) {
            AlarmStatus_t alarm = {.sample_tick=osKernelGetTickCount()};
            Ddm_Evaluate(&ctx.snapshot, &alarm);
            /* TODO: real thresholds/hysteresis/validity before production use.
             * Until thresholds are implemented, alarm remains invalid. */
            const bool changed = !published || alarm.valid != last.valid ||
                alarm.alarm_bits != last.alarm_bits || alarm.warning_bits != last.warning_bits;
            if (SharedData_UpdateAlarm(&alarm)) {
                last = alarm; published = true;
                notify_pending |= changed;
            } else { ++ctx.status.failures; }
        }
        if (notify_pending) {
            if (AppEvents_Notify(APP_TASK_HOST, HOST_EVT_STATUS)) { notify_pending=false; }
            else { ++ctx.status.failures; }
        }
        AppTask_PublishStatus(&ctx, APP_TASK_DDM);
        AppTask_WaitEvents(&ctx, APP_EVT_MODULE, 500U);
    }
}
