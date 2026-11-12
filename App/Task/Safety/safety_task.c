#include "safety_task.h"
#include "global_def.h"

void SafetyTask_Run(void *argument)
{
    (void)argument;

  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
}
