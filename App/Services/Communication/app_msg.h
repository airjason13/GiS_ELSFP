#ifndef APP_MSG_H
#define APP_MSG_H
#include "app_msg_types.h"
#include <stdbool.h>
/* Task-only, nonblocking. True means queued, not completed. Queue ownership
 * transfers even if wakeup fails: consumers also poll; do not resend on true.
 * IDs must not be reused while old results may exist. One in-flight per source. */
bool AppMsg_SendHost(const HostCmdMsg_t *command);
bool AppMsg_SendLaser(const LaserMsg_t *command);
bool AppMsg_SendMsm(const MsmMsg_t *command);
bool AppMsg_SendResult(const ControlResultMsg_t *result);
#endif
