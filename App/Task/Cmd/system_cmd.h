#ifndef SYSTEM_CMD_H
#define SYSTEM_CMD_H

/* Print task status and stack watermarks; Cmd Task context only. */
void SystemCmd_Status(void);
void Cmd_GetSystemVersion(unsigned argc, char *const argv[]);

#endif
