#include "elsfp_all_cmd.h"
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
    if (snapshot.laser_control.valid) {
        debug_printf("[CMD] Laser: requested enable=%u (hardware not applied)\r\n",
                     (unsigned)snapshot.laser_control.enable);
    }
    if (snapshot.laser_control.bias_valid) {
        debug_printf("[CMD] Laser: requested bias=%" PRId32 " uA (hardware not applied)\r\n",
                     (int32_t)snapshot.laser_control.bias_uA);
    } else { debug_printf("[CMD] Laser: bias not configured\r\n"); }
    if (snapshot.laser_control.power_valid) {
        debug_printf("[CMD] Laser: requested power=%" PRId32 " uW (hardware not applied)\r\n",
                     (int32_t)snapshot.laser_control.power_uW);
    } else { debug_printf("[CMD] Laser: power not configured\r\n"); }
    if (snapshot.laser_control.apc_valid) {
        debug_printf("[CMD] Laser: requested apc=%" PRId32 "  (hardware not applied)\r\n",
                     (int32_t)snapshot.laser_control.apc);
    } else { debug_printf("[CMD] Laser: apc not configured\r\n"); }
    if (snapshot.laser.valid) {
        debug_printf("[CMD] Laser: bias=%" PRId32 " uA power=%" PRId32 " uW\r\n",
                     snapshot.laser.bias_uA, snapshot.laser.optical_power_uW);
    } else { debug_printf("[CMD] Laser: not published\r\n"); }
    if (snapshot.tec.valid) {
        debug_printf("[CMD] TEC: temp=%" PRId32 " mdegC current=%" PRId32 " mA\r\n",
                     snapshot.tec.temperature_mdegC, snapshot.tec.current_mA);
    } else { debug_printf("[CMD] TEC: not published\r\n"); }
    debug_printf("[CMD] Fault: valid=%u latched=0x%08" PRIX32 "\r\n",
                 (unsigned)snapshot.fault.valid, snapshot.fault.latched_bits);
}

