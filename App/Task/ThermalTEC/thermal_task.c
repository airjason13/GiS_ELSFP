#include "thermal_task.h"
#include "global_def.h"

/* Called by this Task only. Return true when a sample (valid or invalid)
 * is available for publication. Timestamp must refer to acquisition time. */
static bool Tec_ReadTelemetry(TecTelemetry_t *sample)
{
    (void)sample;
    /* TODO: read actual device/ADC data and fill every field. */
    return false;
}

void ThermalTask_Run(void *argument)
{
    (void)argument;
    static AppTaskRuntime_t ctx;
    for (;;) {
        if (AppTask_ReadSnapshot(&ctx)) {
            /* Observe module/fault snapshot here before real control actions.
             * TODO: implement hardware control and publish measured telemetry here. */
            TecTelemetry_t sample = {0};
            if (Tec_ReadTelemetry(&sample) && !SharedData_UpdateTec(&sample)) {
                ++ctx.status.failures;
            }
        }
        AppTask_PublishStatus(&ctx, APP_TASK_TEC);
        AppTask_WaitEvents(&ctx, APP_EVT_MODULE, 100U);
    }
}
