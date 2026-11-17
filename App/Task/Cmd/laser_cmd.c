#include "laser_cmd.h"
#include "laser_task.h"
#include "cmd_request.h"
#include "platform_log.h"
#include "shared_data.h"
#include <inttypes.h>

static SharedSnapshot_t snapshot;

static bool ReadSnapshot(void)
{
    if (SharedData_GetSnapshot(&snapshot)) { return true; }
    debug_printf("[CMD] shared data unavailable\r\n");
    return false;
}

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

void LaserCmd_Status(unsigned argc, char *const argv[])
{
    (void)argc; (void)argv;
    if (!ReadSnapshot()) { return; }
    if (snapshot.laser_control.valid) {
        debug_printf("[CMD] Laser: requested enable=%u (hardware not applied)\r\n",
                     (unsigned)snapshot.laser_control.enable);
    } else { debug_printf("[CMD] Laser: enable not configured\r\n"); }
    if (snapshot.laser_control.bias_valid) {
        debug_printf("[CMD] Laser: requested bias=%" PRId32 " uA (hardware not applied)\r\n",
                     (int32_t)snapshot.laser_control.bias_uA);
    } else { debug_printf("[CMD] Laser: bias not configured\r\n"); }
    if (snapshot.laser_control.power_valid) {
        debug_printf("[CMD] Laser: requested power=%" PRId32 " uW (hardware not applied)\r\n",
                     (int32_t)snapshot.laser_control.power_uW);
    } else { debug_printf("[CMD] Laser: power not configured\r\n"); }
    if (snapshot.laser_control.apc_valid) {
        debug_printf("[CMD] Laser: requested apc=%" PRId32 "  (hardware not applied)\r\n",
                     (int32_t)snapshot.laser_control.apc);
    } else { debug_printf("[CMD] Laser: apc not configured\r\n"); }
    if (!snapshot.laser.valid) {
        debug_printf("[CMD] Laser: not published\r\n");
        return;
    }
    debug_printf("[CMD] Laser: bias=%" PRId32 " uA power=%" PRId32 " uW\r\n",
                 snapshot.laser.bias_uA,snapshot.laser.optical_power_uW);
}

typedef struct {
    AppOpcode_t opcode;
    int32_t value;
} LaserSetting_t;

/* Module adapter: submit through the Laser API, without receiving results. */
static bool SendLaserSetting(AppTaskId_t source, uint32_t id, const void *context)
{
    const LaserSetting_t *setting=context;
    switch (setting->opcode) {
    case APP_CMD_LASER_ENABLE: return LaserTask_SetEnable(setting->value!=0,source,id);
    case APP_CMD_LASER_BIAS: return LaserTask_SetBias(setting->value,source,id);
    case APP_CMD_LASER_POWER: return LaserTask_SetPower(setting->value,source,id);
    case APP_CMD_LASER_APC: return LaserTask_SetApc(setting->value!=0,source,id);
    default: return false;
    }
}

static void SubmitSetting(AppOpcode_t opcode, const char *field, const char *text, uint32_t maximum)
{
    uint32_t value;
    if (!ParseLaserValue(text,maximum,&value)) {
        debug_printf("[CMD] invalid laser %s: use decimal 0..%" PRIu32 "\r\n",field,maximum);
        return;
    }
    const LaserSetting_t setting={.opcode=opcode,.value=(int32_t)value};
    (void)CmdRequest_Send(opcode,SendLaserSetting,&setting);
}

void LaserCmd_Enable(unsigned argc, char *const argv[])
{
    (void)argc;
    SubmitSetting(APP_CMD_LASER_ENABLE,"enable",argv[0],1U);
}
void LaserCmd_Bias(unsigned argc, char *const argv[])
{
    (void)argc;
    SubmitSetting(APP_CMD_LASER_BIAS,"bias",argv[0],INT32_MAX);
}
void LaserCmd_Power(unsigned argc, char *const argv[])
{
    (void)argc;
    SubmitSetting(APP_CMD_LASER_POWER,"power",argv[0],INT32_MAX);
}
void LaserCmd_Apc(unsigned argc, char *const argv[])
{
    (void)argc;
    SubmitSetting(APP_CMD_LASER_APC,"apc",argv[0],1U);
}
