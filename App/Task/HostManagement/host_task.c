#include "host_task.h"
#include "global_def.h"

void HostMgmtTask_Run(void *argument)
{
    (void)argument;
    for (;;) {
        /* TODO: CMIS parsing, command dispatch and result handling.
         * MSM execution is owned exclusively by MsmTask_Run. */
        osDelay(1);
    }
}
