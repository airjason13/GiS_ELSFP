#include "app_events.h"
#include "app_freertos.h"
static PlatformEventTarget_t target_handle(AppTaskId_t target)
{
    switch (target) {
    case APP_TASK_HOST: return HostMgmtTaskHandle;
    case APP_TASK_MSM: return MsmTaskHandle;
    case APP_TASK_LASER: return LaserApcTaskHandle;
    case APP_TASK_TEC: return ThermalTecTaskHandle;
    case APP_TASK_DDM: return SupervisionDDMTaskHandle;
    case APP_TASK_SAFETY: return SafetyTaskHandle;
    case APP_TASK_STORAGE: return StorageTaskHandle;
    case APP_TASK_DEBUG: return SysdebugTaskHandle;
    default: return NULL;
    }
}
bool AppEvents_Notify(AppTaskId_t target, uint32_t flags)
{ return PlatformEvent_Set(target_handle(target), flags) == PLATFORM_EVENT_OK; }
uint32_t AppEvents_NotifyTargets(uint32_t targets, uint32_t flags)
{
    uint32_t failed = targets;
    for (unsigned i=0; i<APP_TASK_COUNT; ++i) {
        if ((targets & APP_TASK_BIT(i)) != 0U && AppEvents_Notify((AppTaskId_t)i, flags)) {
            failed &= ~APP_TASK_BIT(i);
        }
    }
    return failed;
}
