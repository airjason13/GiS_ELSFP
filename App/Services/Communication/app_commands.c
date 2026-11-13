#include "app_commands.h"
#include "app_events.h"
#include "app_freertos.h"
static bool Send(osMessageQueueId_t queue, const void *message, uint32_t size,
                 AppTaskId_t target, uint32_t event)
{
    if (message == NULL || queue == NULL || osMessageQueueGetMsgSize(queue) != size ||
        osMessageQueuePut(queue, message, 0U, 0U) != osOK) { return false; }
    (void)AppEvents_Notify(target, event); /* Timed polling covers wakeup failure. */
    return true;
}
bool AppCommands_SendHost(const HostCmdMsg_t *c)
{
    return c != NULL && c->header.source < APP_TASK_COUNT &&
           c->length <= HOST_CMD_PAYLOAD_MAX &&
           Send(HostCmdMsgQueueHandle,c,sizeof(*c),APP_TASK_HOST,HOST_EVT_CMD);
}
bool AppCommands_SendLaser(const LaserMsg_t *c)
{ return Send(LaserMsgQueueHandle,c,sizeof(*c),APP_TASK_LASER,APP_EVT_COMMAND); }
bool AppCommands_SendMsm(const MsmMsg_t *c)
{ return Send(MsmMsgQueueHandle,c,sizeof(*c),APP_TASK_MSM,MSM_EVT_INPUT); }
bool AppCommands_SendResult(const ControlResultMsg_t *r)
{ return Send(ControlResultMsgHandle,r,sizeof(*r),APP_TASK_HOST,HOST_EVT_RESULT); }
