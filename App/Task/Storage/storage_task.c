#include "storage_task.h"
#include "global_def.h"

void StorageTask_Run(void *argument)
{
    (void)argument;
    static AppTaskRuntime_t ctx;
    for (;;) {
        if (AppTask_ReadSnapshot(&ctx)) {
            /* Snapshot contains module/fault state for future update admission.
             * TODO: firmware update command processing; no flash writes here. */
        }
        AppTask_PublishStatus(&ctx, APP_TASK_STORAGE);
        AppTask_WaitEvents(&ctx, APP_EVT_MODULE, 100U);
    }
}
