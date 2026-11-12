#include "debug_task.h"
#include "global_def.h"

void DebugTask_Run(void *argument)
{
    (void)argument;

  /* Infinite loop */
  for(;;)
  {
	debug_printf("[DEBUG] Environmental Check Task\n");
    osDelay(osKernelGetTickFreq());
  }
}
