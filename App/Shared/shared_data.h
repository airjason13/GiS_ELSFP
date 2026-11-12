#ifndef SHARED_DATA_H
#define SHARED_DATA_H
#include <stdbool.h>
#include <stdint.h>

typedef enum { SHARED_POWER_UNKNOWN=0, SHARED_POWER_LOW, SHARED_POWER_HIGH } SharedPower_t;
typedef struct {
    uint32_t state; /* Internal MSM state, not CMIS encoding. */
    SharedPower_t power; /* Confirmed mode, not merely requested mode. */
    uint32_t sample_tick;
    bool valid;
} ModuleStatus_t;
typedef struct {
    int32_t bias_uA;
    int32_t optical_power_uW;
    uint32_t sample_tick;
    bool valid;
} LaserTelemetry_t;
typedef struct {
    int32_t temperature_mdegC;
    int32_t current_mA;
    uint32_t sample_tick;
    bool valid;
} TecTelemetry_t;
typedef struct {
    uint32_t alarm_bits;
    uint32_t warning_bits;
    uint32_t sample_tick;
    bool valid;
} AlarmStatus_t;
typedef struct {
    uint32_t latched_bits;
    uint32_t sample_tick;
    bool valid;
} FaultStatus_t;
typedef struct {
    ModuleStatus_t module;
    LaserTelemetry_t laser;
    TecTelemetry_t tec;
    AlarmStatus_t alarm;
    FaultStatus_t fault;
    uint32_t revision; /* Increments on successful writes; wraps naturally. */
} SharedSnapshot_t;

/* Init once before tasks run. Initially every section is invalid.
 * Remaining APIs are task-only, use a finite 10 ms mutex wait, and return
 * false on NULL/uninitialized/lock failure. Never use failed read output.
 * No internal pointers escape. Writers must obey ownership by convention:
 * Module: Host; Laser: LaserAPC; TEC: ThermalTEC; Alarm: Supervision;
 * Fault: Safety only. These APIs do not provide access-control enforcement.
 * sample_tick is caller-supplied RTOS acquisition time, not publish time. */
bool SharedData_Init(void);
bool SharedData_UpdateModule(const ModuleStatus_t *value);
bool SharedData_UpdateLaser(const LaserTelemetry_t *value);
bool SharedData_UpdateTec(const TecTelemetry_t *value);
bool SharedData_UpdateAlarm(const AlarmStatus_t *value);
bool SharedData_GetSnapshot(SharedSnapshot_t *snapshot);
/* Safety must first make hardware safe; publication is not the interlock.
 * On failure retain pending fault bits and retry. Clear only after Safety
 * validates recovery; a Host reset request must not call Clear directly. */
bool SharedData_LatchFault(uint32_t bits, uint32_t sample_tick);
bool SharedData_ClearFault(uint32_t bits, uint32_t sample_tick);
#endif
