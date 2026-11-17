#include "shared_data.h"
#include "platform_rtos.h"
#include <stddef.h>

#define SHARED_LOCK_TIMEOUT_MS 10U
static PlatformMutex_t shared_mutex;
static SharedSnapshot_t shared;

bool SharedData_Init(void)
{
    /* Startup only. Repeated init preserves existing data. */
    if (shared_mutex == NULL) {
        shared_mutex = PlatformMutex_Create("SharedData");
    }
    return shared_mutex != NULL;
}

bool SharedData_UpdateModule(const ModuleStatus_t *value)
{
    if (value == NULL || !PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) {
        return false;
    }
    shared.module = *value;
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}

bool SharedData_UpdateLaser(const LaserTelemetry_t *value)
{
    if (value == NULL || !PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) {
        return false;
    }
    shared.laser = *value;
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}

bool SharedData_UpdateTec(const TecTelemetry_t *value)
{
    if (value == NULL || !PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) {
        return false;
    }
    shared.tec = *value;
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}

bool SharedData_UpdateAlarm(const AlarmStatus_t *value)
{
    if (value == NULL || !PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) {
        return false;
    }
    shared.alarm = *value;
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}

bool SharedData_GetSnapshot(SharedSnapshot_t *snapshot)
{
    if (snapshot == NULL || !PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) {
        return false;
    }
    *snapshot = shared;
    return PlatformMutex_Unlock(shared_mutex);
}

static bool UpdateFault(uint32_t bits, uint32_t sample_tick, bool clear)
{
    if (!PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) {
        return false;
    }
    if (clear) { shared.fault.latched_bits &= ~bits; }
    else { shared.fault.latched_bits |= bits; }
    shared.fault.sample_tick = sample_tick;
    shared.fault.valid = true;
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}

bool SharedData_LatchFault(uint32_t bits, uint32_t sample_tick)
{
    return UpdateFault(bits, sample_tick, false);
}
bool SharedData_ClearFault(uint32_t bits, uint32_t sample_tick)
{
    return UpdateFault(bits, sample_tick, true);
}

bool SharedData_UpdateTask(AppTaskId_t owner, const TaskSharedStatus_t *value)
{
    if ((unsigned)owner >= APP_TASK_COUNT || value == NULL ||
        !PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) { return false; }
    shared.task[owner] = *value;
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}

bool SharedData_UpdateCommandResult(AppTaskId_t owner, const ControlResultMsg_t *result)
{
    if ((unsigned)owner >= APP_TASK_COUNT || result == NULL ||
        !PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) { return false; }
    shared.result[owner].message = *result;
    shared.result[owner].valid = true;
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}

bool SharedData_SetLaserEnable(bool enable)
{
    if (!PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) { return false; }
    shared.laser_control.enable = enable;
    shared.laser_control.valid = true;
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}

bool SharedData_SetLaserBias(int32_t bias_uA)
{
    if (bias_uA < 0) { return false; }
    if (!PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) { return false; }
    shared.laser_control.bias_uA = bias_uA;
    shared.laser_control.bias_valid = true;
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}

bool SharedData_SetLaserPower(int32_t power_uW)
{
    if (power_uW < 0) { return false; }
    if (!PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) { return false; }
    shared.laser_control.power_uW = power_uW;
    shared.laser_control.power_valid = true;
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}

bool SharedData_SetLaserApc(bool enable)
{
    if (!PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) { return false; }
    shared.laser_control.apc = enable;
    shared.laser_control.apc_valid = true;
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}
