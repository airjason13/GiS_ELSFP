#include "thermal_task.h"
#include "global_def.h"

void ThermalTask_Run(void *argument)
{
    (void)argument;

  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
}
