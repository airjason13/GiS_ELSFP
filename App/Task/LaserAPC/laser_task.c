#include "laser_task.h"
#include "global_def.h"

void LaserTask_Run(void *argument)
{
    (void)argument;

  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
}
