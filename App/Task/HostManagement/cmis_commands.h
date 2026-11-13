#ifndef CMIS_COMMANDS_H
#define CMIS_COMMANDS_H
/* Host Task only. Processes normalized internal commands; CMIS register parsing
 * is separate. At most one active command; bounded result draining per call. */
void CmisCommands_ProcessQueues(void);
#endif
