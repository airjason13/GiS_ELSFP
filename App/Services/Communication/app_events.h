#ifndef APP_EVENTS_H
#define APP_EVENTS_H
#include "task_ids.h"
#include "platform_event.h"
#include <stdbool.h>
#define HOST_EVT_CMD       (1UL << 0)
#define HOST_EVT_RESULT    (1UL << 1)
#define HOST_EVT_STATUS    (1UL << 2)
#define HOST_EVT_FAULT     (1UL << 3)
#define APP_EVT_MODULE     (1UL << 4)
#define APP_EVT_COMMAND    (1UL << 6)
#define MSM_EVT_INPUT      (1UL << 5)
#define HOST_EVT_ALL (HOST_EVT_CMD | HOST_EVT_RESULT | HOST_EVT_STATUS | HOST_EVT_FAULT | APP_EVT_MODULE)
#define MSM_EVT_ALL (APP_EVT_MODULE | MSM_EVT_INPUT | HOST_EVT_FAULT)
#define APP_TASK_BIT(id) (1UL << (id))
#define APP_OTHER_TASKS(id) (((1UL << APP_TASK_COUNT) - 1UL) & ~APP_TASK_BIT(id))
bool AppEvents_Notify(AppTaskId_t target, uint32_t flags);
/* Broadcast by setting flags separately on every selected task.
 * Returns failed target bits. Retry those bits later, not an atomic broadcast. */
uint32_t AppEvents_NotifyTargets(uint32_t targets, uint32_t flags);
#endif
