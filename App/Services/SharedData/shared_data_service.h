#ifndef SHARED_DATA_SERVICE_H
#define SHARED_DATA_SERVICE_H
#include <stdbool.h>
#include <stdint.h>
/* Application-facing shared-data helpers. No freshness filtering or delays.
 * Underlying shared-data operations still use their mutex timeout. */
bool SharedDataService_PublishTec(int32_t temperature_mdegC, int32_t current_mA,
                             uint32_t acquired_tick, bool measurement_valid);
bool SharedDataService_PrintSnapshot(void);
/* Call from SafetyTask only, AFTER hardware protection. Keeps pending bits
 * on lock failure; sample_tick is the latest observation time. */
bool SharedDataService_PublishFaults(uint32_t *pending_bits, uint32_t new_bits,
                               uint32_t sample_tick);
#endif
