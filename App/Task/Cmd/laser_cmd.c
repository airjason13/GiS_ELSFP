#include "laser_cmd.h"
#include "laser_task.h"
#include "cmd_request.h"
#include "platform_log.h"
#include "shared_data.h"
#include <inttypes.h>
#include <string.h>

/* Decimal integers only; range is storage capacity, not a hardware limit. */
static bool ParseLaserValue(const char *text, uint32_t maximum, uint32_t *value)
{
    uint32_t parsed=0U;
    if (*text=='\0') { return false; }
    for (; *text!='\0'; ++text) {
        if (*text<'0' || *text>'9') { return false; }
        uint32_t digit=(uint32_t)(*text-'0');
        if (parsed>maximum/10U ||
            (parsed==maximum/10U && digit>maximum%10U)) { return false; }
        parsed=parsed*10U+digit;
    }
    *value=parsed;
    return true;
}

static bool ParseLanes(const char *text, uint8_t *mask)
{
    if (strcmp(text,"all")==0) { *mask=LASER_LANE_ALL; return true; }
    uint32_t lane;
    if (!ParseLaserValue(text,LASER_LANE_COUNT,&lane) || lane==0U) {
        debug_printf("[CMD] invalid lane: use 1..8 or all\r\n"); return false;
    }
    *mask=(uint8_t)(1U<<(lane-1U));
    return true;
}

void LaserCmd_PrintLane(uint8_t lane, const LaserLaneData_t *data)
{
    const LaserRequest_t *c=&data->requested;
    debug_printf("[CMD] Laser Lane %u requested (hardware not applied):\r\n",(unsigned)lane+1U);
    debug_printf("  enable=%u\r\n",(unsigned)c->enable);
    if (c->bias_valid) { debug_printf("  bias=%" PRId32 " uA\r\n",c->bias_uA); }
    else { debug_printf("  bias=not configured\r\n"); }
    if (c->power_valid) { debug_printf("  power=%" PRId32 " uW\r\n",c->power_uW); }
    else { debug_printf("  power=not configured\r\n"); }
    if (c->apc_valid) { debug_printf("  apc=%u\r\n",(unsigned)c->apc); }
    else { debug_printf("  apc=not configured\r\n"); }
    if (data->actual.valid) {
        debug_printf("  actual enabled=%u apc=%u\r\n",(unsigned)data->actual.enabled,(unsigned)data->actual.apc_active);
    } else { debug_printf("  actual status: not published\r\n"); }
    if (data->measured.valid) {
        debug_printf("  measured bias=%" PRId32 " uA power=%" PRId32 " uW\r\n",
                     data->measured.bias_uA,data->measured.optical_power_uW);
    } else { debug_printf("  Laser: not published\r\n"); }
}

void Cmd_GetLaserStatus(unsigned argc, char *const argv[])
{
    (void)argc;
    uint8_t mask;
    if (!ParseLanes(argv[0],&mask)) { return; }
    for (uint8_t lane=0; lane<LASER_LANE_COUNT; ++lane) {
        if ((mask & (1U<<lane))==0U) { continue; }
        LaserLaneData_t data;
        if (!SharedData_GetLaserLane(lane,&data)) {
            debug_printf("[CMD] shared data unavailable\r\n"); return;
        }
        LaserCmd_PrintLane(lane,&data);
    }
}

typedef struct {
    AppOpcode_t opcode;
    uint8_t lane_mask;
    int32_t value;
} LaserSetting_t;

/* Module adapter: submit through the Laser API, without receiving results. */
static bool SendLaserSetting(AppTaskId_t source, uint32_t id, const void *context)
{
    const LaserSetting_t *setting=context;
    switch (setting->opcode) {
    case APP_CMD_LASER_ENABLE: return LaserTask_SetEnable(setting->lane_mask,setting->value!=0,source,id);
    case APP_CMD_LASER_BIAS: return LaserTask_SetBias(setting->lane_mask,setting->value,source,id);
    case APP_CMD_LASER_POWER: return LaserTask_SetPower(setting->lane_mask,setting->value,source,id);
    case APP_CMD_LASER_APC: return LaserTask_SetApc(setting->lane_mask,setting->value!=0,source,id);
    default: return false;
    }
}

static void SubmitSetting(AppOpcode_t opcode, const char *field, const char *lanes, const char *text, uint32_t maximum)
{
    uint8_t mask;
    if (!ParseLanes(lanes,&mask)) { return; }
    uint32_t value;
    if (!ParseLaserValue(text,maximum,&value)) {
        debug_printf("[CMD] invalid laser %s: use decimal 0..%" PRIu32 "\r\n",field,maximum);
        return;
    }
    const LaserSetting_t setting={.opcode=opcode,.lane_mask=mask,.value=(int32_t)value};
    (void)CmdRequest_Send(opcode,SendLaserSetting,&setting);
}

void Cmd_SetLaserEnable(unsigned argc, char *const argv[])
{
    (void)argc;
    SubmitSetting(APP_CMD_LASER_ENABLE,"enable",argv[0],argv[1],1U);
}
void Cmd_SetLaserBias(unsigned argc, char *const argv[])
{
    (void)argc;
    SubmitSetting(APP_CMD_LASER_BIAS,"bias",argv[0],argv[1],INT32_MAX);
}
void Cmd_SetLaserPower(unsigned argc, char *const argv[])
{
    (void)argc;
    SubmitSetting(APP_CMD_LASER_POWER,"power",argv[0],argv[1],INT32_MAX);
}
void Cmd_SetLaserApc(unsigned argc, char *const argv[])
{
    (void)argc;
    SubmitSetting(APP_CMD_LASER_APC,"apc",argv[0],argv[1],1U);
}
