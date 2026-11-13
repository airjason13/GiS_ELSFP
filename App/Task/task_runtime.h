#ifndef APP_TASK_RUNTIME_H
#define APP_TASK_RUNTIME_H
#include "shared_data.h"
#include "app_events.h"
/* Declare STATIC per task: snapshot is intentionally kept off small stacks. */
typedef struct {
    SharedSnapshot_t snapshot;
    TaskSharedStatus_t status;
    uint32_t events;
    bool module_event_pending;
} AppTaskRuntime_t;
bool AppTask_ReadSnapshot(AppTaskRuntime_t *ctx);
void AppTask_PublishStatus(AppTaskRuntime_t *ctx, AppTaskId_t owner);
void AppTask_WaitEvents(AppTaskRuntime_t *ctx, uint32_t mask, uint32_t timeout_ms);
#endif
