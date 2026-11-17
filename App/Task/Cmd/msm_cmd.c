#include "msm_cmd.h"
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

void MsmCmd_Status(void)
{
    if (ReadSnapshot()) { PrintModule(); }
}

