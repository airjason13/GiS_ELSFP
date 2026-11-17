#include "laser_task.h"
#include "global_def.h"
#include "app_msg.h"
#include "app_freertos.h"
#include <string.h>

/* Called by this Task only. Return true when a sample (valid or invalid)
 * is available for publication. Timestamp must refer to acquisition time. */
static bool Laser_ReadTelemetry(LaserTelemetry_t *sample)
{
    (void)sample;
    /* TODO: read actual device/ADC data and fill every field. */
    return false;
}

/* Runs only in Laser Task. Staging settings is permitted before HW is ready.
 * No hardware enable is performed here. Future hardware application must check
 * current MSM/fault permission, device limits and publish actual status. */
static int32_t Laser_Execute(const LaserMsg_t *command)
{
    bool stored;
    if (command->target<0) { return APP_RESULT_INVALID; }
    switch (command->header.opcode) {
    case APP_CMD_LASER_ENABLE:
        if (command->target>1) { return APP_RESULT_INVALID; }
        stored=SharedData_SetLaserEnable(command->target!=0); break;
    case APP_CMD_LASER_BIAS:
        stored=SharedData_SetLaserBias(command->target); break;
    case APP_CMD_LASER_POWER:
        stored=SharedData_SetLaserPower(command->target); break;
    case APP_CMD_LASER_APC:
        if (command->target>1) { return APP_RESULT_INVALID; }
        stored=SharedData_SetLaserApc(command->target!=0); break;
    default: return APP_RESULT_UNSUPPORTED;
    }
    return stored ? APP_RESULT_STORED : APP_RESULT_BUSY;
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
        if (!result_pending && osMessageQueueGet(LaserRequestQueueHandle,&command,NULL,0U)==osOK) {
            result=(ControlResultMsg_t){.header=command.header,
                .status=Laser_Execute(&command), .detail=command.target};
            result.header.source=APP_TASK_LASER;
            result_pending=true; /* Retry result delivery, never repeat the operation. */
        }
        if (result_pending && AppMsg_SendResult(&result)) { result_pending=false; }
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

/* These APIs run in the caller context and submit through Host arbitration. */
static bool Submit(AppTaskId_t source, uint32_t request_id, AppOpcode_t opcode, int32_t value)
{
    if (value<0) { return false; }
    HostCmdMsg_t request={.header={.request_id=request_id,.opcode=opcode,.source=(uint16_t)source},
                          .length=sizeof(value)};
    memcpy(request.data,&value,sizeof(value));
    return AppMsg_SendHost(&request);
}

bool LaserTask_SetEnable(bool enable, AppTaskId_t source, uint32_t request_id)
{
    return Submit(source,request_id,APP_CMD_LASER_ENABLE,enable);
}

bool LaserTask_SetBias(int32_t bias_uA, AppTaskId_t source, uint32_t request_id)
{
    return Submit(source,request_id,APP_CMD_LASER_BIAS,bias_uA);
}

bool LaserTask_SetPower(int32_t power_uW, AppTaskId_t source, uint32_t request_id)
{
    return Submit(source,request_id,APP_CMD_LASER_POWER,power_uW);
}

bool LaserTask_SetApc(bool enable, AppTaskId_t source, uint32_t request_id)
{
    return Submit(source,request_id,APP_CMD_LASER_APC,enable);
}
