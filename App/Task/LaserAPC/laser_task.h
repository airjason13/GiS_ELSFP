#ifndef APP_LASER_TASK_H
#define APP_LASER_TASK_H
#include <stdbool.h>
#include <stdint.h>
#include "task_ids.h"

/* Task-only submission through Host. True means queued, NOT applied.
 * Caller supplies unique IDs and allows only one outstanding request per source.
 * Read completion from the source mailbox; only Host consumes result queue. */
bool LaserTask_SetEnable(uint8_t lane_mask, bool enable, AppTaskId_t source, uint32_t request_id);

bool LaserTask_SetBias(uint8_t lane_mask, int32_t bias_uA, AppTaskId_t source, uint32_t request_id);

bool LaserTask_SetPower(uint8_t lane_mask, int32_t power_uW, AppTaskId_t source, uint32_t request_id);

bool LaserTask_SetApc(uint8_t lane_mask, bool enable, AppTaskId_t source, uint32_t request_id);

/* Runs in the existing CubeMX-created task; does not return. */
void LaserTask_Run(void *argument);

#endif /* APP_LASER_TASK_H */
