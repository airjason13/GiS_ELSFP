#include "elsfp_all_cmd.h"
#include "laser_cmd.h"
#include "platform_log.h"
#include "shared_data.h"
#include "module_msm.h"
#include <inttypes.h>

static SharedSnapshot_t snapshot;

static bool ReadSnapshot(void)
{
    if (SharedData_GetSnapshot(&snapshot)) { return true; }
    debug_printf("[CMD] shared data unavailable\r\n");
    return false;
}

static void PrintModule(void)
{
    if (!snapshot.module.valid) {
        debug_printf("[CMD] module: not published\r\n");
        return;
    }
    debug_printf("[CMD] MSM=%s seq=%" PRIu32 " power=%u\r\n",
                 ModuleMsm_StateName((MsmState_t)snapshot.module.state),
                 snapshot.module.broadcast_sequence, (unsigned)snapshot.module.power);
}

void ElsfpAllCmd_Status(void)
{
    if (!ReadSnapshot()) { return; }
    PrintModule();
    for (uint8_t lane=0; lane<LASER_LANE_COUNT; ++lane) {
        LaserCmd_PrintLane(lane,&snapshot.laser[lane]);
    }
    if (snapshot.tec.valid) {
        debug_printf("[CMD] TEC: temp=%" PRId32 " mdegC current=%" PRId32 " mA\r\n",
                     snapshot.tec.temperature_mdegC, snapshot.tec.current_mA);
    } else { debug_printf("[CMD] TEC: not published\r\n"); }
    debug_printf("[CMD] Fault: valid=%u latched=0x%08" PRIX32 "\r\n",
                 (unsigned)snapshot.fault.valid, snapshot.fault.latched_bits);
}

