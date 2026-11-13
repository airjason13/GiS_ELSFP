#ifndef SHARED_DATA_H
#define SHARED_DATA_H
#include <stdbool.h>
#include <stdint.h>
#include "task_ids.h"
#include "messages_queue.h"

typedef enum { SHARED_POWER_UNKNOWN=0, SHARED_POWER_LOW, SHARED_POWER_HIGH } SharedPower_t;
typedef struct {
    uint32_t state;
    uint32_t broadcast_sequence;
    SharedPower_t power;
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
    uint32_t iterations;
    uint32_t last_events;
    uint32_t observed_module_state;
    uint32_t module_event_sequence;
    uint32_t sample_tick;
    uint32_t failures;
    bool valid;
} TaskSharedStatus_t;
typedef struct {
    ControlResultMsg_t message;
    bool valid;
} CommandResultStatus_t;

typedef struct {
    /* Host writes completion mailboxes; one outstanding command per source. */
    CommandResultStatus_t result[APP_TASK_COUNT];
    TaskSharedStatus_t task[APP_TASK_COUNT];
    ModuleStatus_t module;
    LaserTelemetry_t laser;
    TecTelemetry_t tec;
    AlarmStatus_t alarm;
    FaultStatus_t fault;
    uint32_t revision;
} SharedSnapshot_t;

/* Init once before tasks run. Initially every section is invalid.
 * Remaining APIs are task-only, use a finite 10 ms mutex wait, and return
 * false on NULL/uninitialized/lock failure. Never use failed read output.
 * No internal pointers escape. Writers must obey ownership by convention:
 * Module: MSM; Laser: LaserAPC; TEC: ThermalTEC; Alarm: Supervision;
 * Fault: Safety only. These APIs do not provide access-control enforcement.
 * sample_tick is caller-supplied RTOS acquisition time, not publish time. */
bool SharedData_Init(void);
/* Host only. Result header.source identifies the worker, owner the requester. */
bool SharedData_UpdateCommandResult(AppTaskId_t owner, const ControlResultMsg_t *result);
/* Caller writes only its own task slot. Ownership is a code convention. */
bool SharedData_UpdateTask(AppTaskId_t owner, const TaskSharedStatus_t *value);
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
