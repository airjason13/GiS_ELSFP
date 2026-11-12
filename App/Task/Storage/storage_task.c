#include "storage_task.h"
#include "global_def.h"

void StorageTask_Run(void *argument)
{
    (void)argument;

  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
}
