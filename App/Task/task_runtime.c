#include "task_runtime.h"
#include "cmsis_os2.h"
bool AppTask_ReadSnapshot(AppTaskRuntime_t *ctx)
{
    ++ctx->status.iterations;
    if (!SharedData_GetSnapshot(&ctx->snapshot)) {
        ++ctx->status.failures;
        return false;
    }
    if (ctx->snapshot.module.valid) {
        ctx->status.observed_module_state = ctx->snapshot.module.state;
        if (ctx->module_event_pending) {
            ctx->status.module_event_sequence = ctx->snapshot.module.broadcast_sequence;
            ctx->module_event_pending = false;
        }
    }
    return true;
}
void AppTask_PublishStatus(AppTaskRuntime_t *ctx, AppTaskId_t owner)
{
    ctx->status.sample_tick = osKernelGetTickCount();
    ctx->status.valid = true;
    if (!SharedData_UpdateTask(owner, &ctx->status)) { ++ctx->status.failures; }
}
void AppTask_WaitEvents(AppTaskRuntime_t *ctx, uint32_t mask, uint32_t timeout_ms)
{
    PlatformEventStatus_t result = PlatformEvent_Wait(mask, timeout_ms, &ctx->events);
    if (result == PLATFORM_EVENT_OK) {
        ctx->status.last_events = ctx->events;
        ctx->module_event_pending |= (ctx->events & APP_EVT_MODULE) != 0U;
    }
    else if (result != PLATFORM_EVENT_TIMEOUT) {
        ++ctx->status.failures;
        osDelay(1); /* Avoid a runaway loop if an invalid context/config occurs. */
    }
}
