#include "supervision_task.h"
#include "global_def.h"

void SupervisionTask_Run(void *argument)
{
    (void)argument;

  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
}
