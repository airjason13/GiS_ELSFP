#ifndef LASER_CMD_H
#define LASER_CMD_H
#include "shared_data.h"

void LaserCmd_PrintLane(uint8_t lane, const LaserLaneData_t *data);

/* Cmd Task handlers; argc/argv contain only value arguments. */
void Cmd_GetLaserStatus(unsigned argc, char *const argv[]);
void Cmd_SetLaserEnable(unsigned argc, char *const argv[]);
void Cmd_SetLaserBias(unsigned argc, char *const argv[]);
void Cmd_SetLaserPower(unsigned argc, char *const argv[]);
void Cmd_SetLaserApc(unsigned argc, char *const argv[]);

#endif
