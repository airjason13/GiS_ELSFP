#include "shared_data_service.h"
#include "shared_data.h"
#include "platform_log.h"
#include <inttypes.h>
#include <stddef.h>

bool SharedDataService_PublishTec(int32_t temperature_mdegC, int32_t current_mA,
                             uint32_t acquired_tick, bool measurement_valid)
{
    const TecTelemetry_t sample = {
        .temperature_mdegC = temperature_mdegC,
        .current_mA = current_mA,
        .sample_tick = acquired_tick,
        .valid = measurement_valid
    };
    return SharedData_UpdateTec(&sample);
}

bool SharedDataService_PrintSnapshot(void)
{
    SharedSnapshot_t snapshot;
    if (!SharedData_GetSnapshot(&snapshot)) {
        return false; /* Never consume an unsuccessful read. */
    }
    /* Snapshot copy is complete; logging happens WITHOUT the shared mutex. */
    if (!snapshot.tec.valid) {
        return debug_printf("[Shared] TEC data invalid\r\n") >= 0;
    }
    return debug_printf("[Shared] rev=%" PRIu32 " temp=%" PRId32
                        " mdegC current=%" PRId32 " mA\r\n",
                        snapshot.revision, snapshot.tec.temperature_mdegC,
                        snapshot.tec.current_mA) >= 0;
}

bool SharedDataService_PublishFaults(uint32_t *pending_bits, uint32_t new_bits,
                               uint32_t sample_tick)
{
    if (pending_bits == NULL) { return false; }
    *pending_bits |= new_bits;
    if (*pending_bits == 0U) { return true; }
    if (!SharedData_LatchFault(*pending_bits, sample_tick)) {
        return false; /* Retry later from SafetyTask, without delaying shutdown. */
    }
    *pending_bits = 0U;
    return true;
}
