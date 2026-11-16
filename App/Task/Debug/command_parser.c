#include "command_parser.h"
#include "platform_log.h"
#include "shared_data.h"
#include "module_msm.h"
#include <inttypes.h>
#include <stddef.h>
#include <string.h>

#define COMMAND_ARGS_MAX 8U
#define CMD_PATH_LENGTH 3U
/* Single Debug Task caller; static storage avoids large stack allocations. */
static char command_buffer[COMMAND_LINE_MAX + 1U];
static char *arguments[COMMAND_ARGS_MAX];
static SharedSnapshot_t snapshot;

/* Handlers receive only the value arguments after verb/target/field. */
typedef void (*CmdHandler_t)(unsigned argc, char *const argv[]);
typedef struct {
    const char *verb;
    const char *target;
    const char *field;
    unsigned value_count;
    CmdHandler_t handler;
    const char *value_usage;
    const char *description;
} CmdEntry_t;

static void Cmd_GetElsfpStatus(unsigned argc, char *const argv[]);
static void Cmd_GetMsmStatus(unsigned argc, char *const argv[]);

/* Add one row + one handler. Dispatch and help both use this table.
 * For setters, value_count counts arguments AFTER the three-word path;
 * value_usage describes them, e.g. "<mW>". Validate values in the handler. */
static const CmdEntry_t commands[] = {
    /* Verb   Target   Field     Values  Handler             Value help  Description */
    { "get", "elsfp", "status", 0U,     Cmd_GetElsfpStatus,  "",         "Show module, telemetry and faults" },
    { "get", "msm",   "status", 0U,     Cmd_GetMsmStatus,    "",         "Show current module state" }
};
#define CMD_COUNT (sizeof(commands) / sizeof(commands[0]))

static const char *PathWord(const CmdEntry_t *entry, unsigned index)
{
    return index==0U ? entry->verb : index==1U ? entry->target : entry->field;
}

static bool Matches(const CmdEntry_t *entry, unsigned count, char *const words[])
{
    for (unsigned i=0U; i<count; ++i) {
        if (strcmp(PathWord(entry,i),words[i])!=0) { return false; }
    }
    return true;
}

static bool HasVerb(const char *verb)
{
    for (unsigned i=0U; i<CMD_COUNT; ++i) {
        if (strcmp(commands[i].verb,verb)==0) { return true; }
    }
    return false;
}

static void PrintUsage(const CmdEntry_t *entry)
{
    debug_printf("  %s %s %s%s%s  %s\r\n",entry->verb,entry->target,entry->field,
                 entry->value_count!=0U ? " " : "",entry->value_usage,entry->description);
}

/* Show only the next level; never execute a handler from help. */
static void ShowHelp(unsigned count, char *const words[])
{
    if (count==0U) {
        debug_printf("Commands:\r\n  help  Show command help\r\n");
        for (unsigned i=0U; i<CMD_COUNT; ++i) {
            bool seen=false;
            for (unsigned j=0U; j<i; ++j) {
                if (strcmp(commands[j].verb,commands[i].verb)==0) { seen=true; break; }
            }
            if (!seen) { debug_printf("  %s\r\n",commands[i].verb); }
        }
        if (!HasVerb("set")) { debug_printf("  set   Change settings (not available yet)\r\n"); }
        debug_printf("Usage: help <command>\r\n");
        return;
    }
    if (strcmp(words[0],"set")==0 && !HasVerb("set")) {
        debug_printf("[CMD] No settings available; no setting changed\r\n");
        return;
    }
    if (count>CMD_PATH_LENGTH || !HasVerb(words[0])) {
        debug_printf("[CMD] usage: help | help <command> [target] [field]\r\n");
        return;
    }
    bool found=false;
    if (count==1U) { debug_printf("Targets:\r\n"); }
    for (unsigned i=0U; i<CMD_COUNT; ++i) {
        if (!Matches(&commands[i],count,words)) { continue; }
        found=true;
        if (count==1U) {
            bool seen=false;
            for (unsigned j=0U; j<i; ++j) {
                if (Matches(&commands[j],1U,words) && strcmp(commands[j].target,commands[i].target)==0) {
                    seen=true; break;
                }
            }
            if (!seen) { debug_printf("  %s\r\n",commands[i].target); }
        } else { PrintUsage(&commands[i]); }
    }
    if (!found) {
        if (count==2U) { debug_printf("[CMD] unknown %s target: %s\r\n",words[0],words[1]); }
        else { debug_printf("[CMD] usage: help <command> <target> [field]\r\n"); }
        ShowHelp(count-1U,words);
    } else if (count==1U) { debug_printf("Usage: help %s <target>\r\n",words[0]); }
}

static void Dispatch(unsigned argc, char *const argv[])
{
    if (strcmp(argv[0],"help")==0) { ShowHelp(argc-1U,argv+1); return; }
    if (strcmp(argv[0],"set")==0 && !HasVerb("set")) { ShowHelp(1U,argv); return; }
    if (!HasVerb(argv[0])) {
        debug_printf("[CMD] unknown command: %s (type help)\r\n",argv[0]);
        return;
    }
    if (argc<CMD_PATH_LENGTH) { ShowHelp(argc,argv); return; }
    for (unsigned i=0U; i<CMD_COUNT; ++i) {
        const CmdEntry_t *entry=&commands[i];
        if (!Matches(entry,CMD_PATH_LENGTH,argv)) { continue; }
        if (argc-CMD_PATH_LENGTH!=entry->value_count) {
            debug_printf("[CMD] invalid %s arguments\r\n",argv[0]);
            PrintUsage(entry);
            return;
        }
        entry->handler(argc-CMD_PATH_LENGTH,argv+CMD_PATH_LENGTH);
        return;
    }
    debug_printf("[CMD] invalid %s arguments\r\n",argv[0]);
    ShowHelp(2U,argv);
}

static bool ReadSnapshot(void)
{
    if (SharedData_GetSnapshot(&snapshot)) { return true; }
    debug_printf("[CMD] shared data unavailable\r\n");
    return false;
}

static void PrintModule(void)
{
    if (!snapshot.module.valid) {
        debug_printf("[CMD] module: not published\r\n");
        return;
    }
    debug_printf("[CMD] MSM=%s seq=%" PRIu32 " power=%u\r\n",
                 ModuleMsm_StateName((MsmState_t)snapshot.module.state),
                 snapshot.module.broadcast_sequence, (unsigned)snapshot.module.power);
}

static void Cmd_GetElsfpStatus(unsigned argc, char *const argv[])
{
    (void)argc;
    (void)argv;
    if (!ReadSnapshot()) { return; }
    PrintModule();
    if (snapshot.laser.valid) {
        debug_printf("[CMD] Laser: bias=%" PRId32 " uA power=%" PRId32 " uW\r\n",
                     snapshot.laser.bias_uA, snapshot.laser.optical_power_uW);
    } else { debug_printf("[CMD] Laser: not published\r\n"); }
    if (snapshot.tec.valid) {
        debug_printf("[CMD] TEC: temp=%" PRId32 " mdegC current=%" PRId32 " mA\r\n",
                     snapshot.tec.temperature_mdegC, snapshot.tec.current_mA);
    } else { debug_printf("[CMD] TEC: not published\r\n"); }
    debug_printf("[CMD] Fault: valid=%u latched=0x%08" PRIX32 "\r\n",
                 (unsigned)snapshot.fault.valid, snapshot.fault.latched_bits);
}

static void Cmd_GetMsmStatus(unsigned argc, char *const argv[])
{
    (void)argc;
    (void)argv;
    if (ReadSnapshot()) { PrintModule(); }
}

void CommandParser_Parse(const char *line)
{
    if (line==NULL) { debug_printf("[CMD] invalid input\r\n"); return; }
    unsigned length=0U;
    while (length<=COMMAND_LINE_MAX && line[length]!='\0') { ++length; }
    if (length>COMMAND_LINE_MAX) {
        debug_printf("[CMD] line too long (max %u)\r\n", (unsigned)COMMAND_LINE_MAX);
        return;
    }
    memcpy(command_buffer,line,length+1U);
    unsigned argc=0U;
    char *cursor=command_buffer;
    while (*cursor!='\0') {
        while (*cursor==' ' || *cursor=='\t') { ++cursor; }
        if (*cursor=='\0') { break; }
        if (argc==COMMAND_ARGS_MAX) {
            debug_printf("[CMD] too many arguments (max 8 tokens)\r\n");
            return;
        }
        arguments[argc++]=cursor;
        while (*cursor!='\0' && *cursor!=' ' && *cursor!='\t') {
            if ((unsigned char)*cursor<32U || (unsigned char)*cursor>126U) {
                debug_printf("[CMD] invalid character\r\n");
                return;
            }
            ++cursor;
        }
        if (*cursor!='\0') { *cursor++='\0'; }
    }
    if (argc!=0U) { Dispatch(argc,arguments); }
}
