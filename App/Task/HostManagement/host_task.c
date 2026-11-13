#include "host_task.h"
#include "global_def.h"
#include "cmis_commands.h"

void HostMgmtTask_Run(void *argument)
{
    (void)argument;
    static AppTaskRuntime_t ctx;
    uint32_t pending = 0U;
    for (;;) {
        CmisCommands_ProcessQueues();
        pending |= ctx.events;
        if (AppTask_ReadSnapshot(&ctx)) {
            /* Independent branches: multiple event bits may arrive together.
             * Failed snapshots retain pending events for retry on next cycle. */
            if ((pending & HOST_EVT_FAULT) != 0U) {
                debug_printf("[HOST] Safety sync: valid=%u fault=0x%08" PRIx32 "\r\n",
                    (unsigned)ctx.snapshot.fault.valid, ctx.snapshot.fault.latched_bits);
            }
            if ((pending & HOST_EVT_STATUS) != 0U) {
                debug_printf("[HOST] DDM sync: valid=%u alarm=0x%08" PRIx32
                             " warning=0x%08" PRIx32 "\r\n",
                    (unsigned)ctx.snapshot.alarm.valid, ctx.snapshot.alarm.alarm_bits,
                    ctx.snapshot.alarm.warning_bits);
            }
            if ((pending & APP_EVT_MODULE) != 0U) {
                debug_printf("[HOST] MSM sync: state=%" PRIu32 "\r\n",
                    ctx.snapshot.module.state);
            }
            /* TODO: map current state/results into CMIS Register Shadow. */
            pending = 0U;
        }
        AppTask_PublishStatus(&ctx, APP_TASK_HOST);
        AppTask_WaitEvents(&ctx, HOST_EVT_ALL, 100U);
    }
}
