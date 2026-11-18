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

bool SharedData_UpdateLaserMeasured(uint8_t lane, const LaserTelemetry_t *value)
{
    if (lane >= LASER_LANE_COUNT || value == NULL || !PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) {
        return false;
    }
    shared.laser[lane].measured = *value;
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

bool SharedData_SetLaserRequestedEnable(uint8_t lane_mask, bool enable)
{
    if (lane_mask==0U) { return false; }
    if (!PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) { return false; }
    /* Update every selected lane under one lock. */
    for (unsigned lane=0; lane<LASER_LANE_COUNT; ++lane) {
        if ((lane_mask & (1U<<lane))==0U) { continue; }
        shared.laser[lane].requested.enable=enable;
    }
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}

bool SharedData_SetLaserRequestedBias(uint8_t lane_mask, int32_t bias_uA)
{
    if (lane_mask==0U || bias_uA<0) { return false; }
    if (!PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) { return false; }
    /* Update every selected lane under one lock. */
    for (unsigned lane=0; lane<LASER_LANE_COUNT; ++lane) {
        if ((lane_mask & (1U<<lane))==0U) { continue; }
        shared.laser[lane].requested.bias_uA=bias_uA;
        shared.laser[lane].requested.bias_valid=true;
    }
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}

bool SharedData_SetLaserRequestedPower(uint8_t lane_mask, int32_t power_uW)
{
    if (lane_mask==0U || power_uW<0) { return false; }
    if (!PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) { return false; }
    /* Update every selected lane under one lock. */
    for (unsigned lane=0; lane<LASER_LANE_COUNT; ++lane) {
        if ((lane_mask & (1U<<lane))==0U) { continue; }
        shared.laser[lane].requested.power_uW=power_uW;
        shared.laser[lane].requested.power_valid=true;
    }
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}

bool SharedData_SetLaserRequestedApc(uint8_t lane_mask, bool enable)
{
    if (lane_mask==0U) { return false; }
    if (!PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) { return false; }
    /* Update every selected lane under one lock. */
    for (unsigned lane=0; lane<LASER_LANE_COUNT; ++lane) {
        if ((lane_mask & (1U<<lane))==0U) { continue; }
        shared.laser[lane].requested.apc=enable;
        shared.laser[lane].requested.apc_valid=true;
    }
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}

bool SharedData_GetLaserLane(uint8_t lane, LaserLaneData_t *value)
{
    if (lane>=LASER_LANE_COUNT || value==NULL ||
        !PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) { return false; }
    *value=shared.laser[lane];
    return PlatformMutex_Unlock(shared_mutex);
}
bool SharedData_UpdateLaserActual(uint8_t lane, const LaserStatus_t *value)
{
    if (lane>=LASER_LANE_COUNT || value==NULL ||
        !PlatformMutex_Lock(shared_mutex, SHARED_LOCK_TIMEOUT_MS)) { return false; }
    shared.laser[lane].actual=*value;
    ++shared.revision;
    return PlatformMutex_Unlock(shared_mutex);
}
