#ifndef CMD_REQUEST_H
#define CMD_REQUEST_H
#include "app_msg_types.h"
#include "task_ids.h"
#include <stdbool.h>

/* Called synchronously. Copy request data into a queue; never retain context. */
typedef bool (*CmdRequestSubmit_t)(AppTaskId_t source, uint32_t request_id,
                                  const void *context);
/* Cmd Task only. One outstanding request across all command modules. */
bool CmdRequest_Send(AppOpcode_t opcode, CmdRequestSubmit_t submit, const void *context);
/* Read the Host-published mailbox; never consume the worker result queue. */
void CmdRequest_ProcessResult(void);
#endif
