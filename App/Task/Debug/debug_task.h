#ifndef APP_DEBUG_TASK_H
#define APP_DEBUG_TASK_H

/* Debug feature switches: 0 = off, 1 = on. */
#ifndef DEBUG_SYS_INFO
#define DEBUG_SYS_INFO 0
#endif

/* Runs in the existing CubeMX-created task; does not return. */
void DebugTask_Run(void *argument);

#endif /* APP_DEBUG_TASK_H */
