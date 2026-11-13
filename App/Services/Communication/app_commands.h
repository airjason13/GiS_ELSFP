#ifndef APP_COMMANDS_H
#define APP_COMMANDS_H
#include "messages_queue.h"
#include <stdbool.h>
/* Task-only, nonblocking. True means queued, not completed. Queue ownership
 * transfers even if wakeup fails: consumers also poll; do not resend on true.
 * IDs must not be reused while old results may exist. One in-flight per source. */
bool AppCommands_SendHost(const HostCmdMsg_t *command);
bool AppCommands_SendLaser(const LaserMsg_t *command);
bool AppCommands_SendMsm(const MsmMsg_t *command);
bool AppCommands_SendResult(const ControlResultMsg_t *result);
#endif
