#ifndef LASER_CMD_H
#define LASER_CMD_H

/* Cmd Task handlers; argc/argv contain only value arguments. */
void LaserCmd_Status(unsigned argc, char *const argv[]);
void LaserCmd_Enable(unsigned argc, char *const argv[]);
void LaserCmd_Bias(unsigned argc, char *const argv[]);
void LaserCmd_Power(unsigned argc, char *const argv[]);
void LaserCmd_Apc(unsigned argc, char *const argv[]);

#endif
