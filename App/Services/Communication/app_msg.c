#include "app_msg.h"
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
bool AppMsg_SendHost(const HostCmdMsg_t *c)
{
    return c != NULL && c->header.source < APP_TASK_COUNT &&
           c->length <= HOST_CMD_PAYLOAD_MAX &&
           Send(HostRequestQueueHandle,c,sizeof(*c),APP_TASK_HOST,HOST_EVT_CMD);
}
bool AppMsg_SendLaser(const LaserMsg_t *c)
{ return Send(LaserRequestQueueHandle,c,sizeof(*c),APP_TASK_LASER,APP_EVT_COMMAND); }
bool AppMsg_SendMsm(const MsmMsg_t *c)
{ return Send(MsmMsgQueueHandle,c,sizeof(*c),APP_TASK_MSM,MSM_EVT_INPUT); }

bool AppMsg_SendResult(const ControlResultMsg_t *r)
{ return Send(WorkerResultQueueHandle,r,sizeof(*r),APP_TASK_HOST,HOST_EVT_RESULT); }
