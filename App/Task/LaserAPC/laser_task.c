#include "laser_task.h"
#include "global_def.h"
#include "app_commands.h"
#include "app_freertos.h"

/* Called by this Task only. Return true when a sample (valid or invalid)
 * is available for publication. Timestamp must refer to acquisition time. */
static bool Laser_ReadTelemetry(LaserTelemetry_t *sample)
{
    (void)sample;
    /* TODO: read actual device/ADC data and fill every field. */
    return false;
}

void LaserTask_Run(void *argument)
{
    (void)argument;
    static AppTaskRuntime_t ctx;
    static ControlResultMsg_t result;
    bool result_pending=false;
    for (;;) {
        /* One command per iteration; retain reply when the result queue is full. */
        LaserMsg_t command;
        if (!result_pending && osMessageQueueGet(LaserMsgQueueHandle,&command,NULL,0U)==osOK) {
            result=(ControlResultMsg_t){.header=command.header,
                .status=command.header.opcode==APP_CMD_LASER_PING ? APP_RESULT_OK : APP_RESULT_UNSUPPORTED};
            result.header.source=APP_TASK_LASER;
            result_pending=true; /* PING checks routing only; no laser hardware action. */
        }
        if (result_pending && AppCommands_SendResult(&result)) { result_pending=false; }
        if (AppTask_ReadSnapshot(&ctx)) {
            /* Observe module/fault snapshot here before real control actions.
             * TODO: implement hardware control and publish measured telemetry here. */
            LaserTelemetry_t sample = {0};
            if (Laser_ReadTelemetry(&sample) && !SharedData_UpdateLaser(&sample)) {
                ++ctx.status.failures;
            }
        }

        AppTask_PublishStatus(&ctx, APP_TASK_LASER);
        AppTask_WaitEvents(&ctx, APP_EVT_MODULE | APP_EVT_COMMAND, 100U);
    }
}
