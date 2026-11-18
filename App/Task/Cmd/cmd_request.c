#include "cmd_request.h"
#include "shared_data.h"
#include "platform_log.h"
#include <inttypes.h>
#include <stddef.h>

static bool pending;
static uint32_t next_id=0x80000000U, pending_id;
static AppOpcode_t pending_opcode;

bool CmdRequest_Send(AppOpcode_t opcode, CmdRequestSubmit_t submit, const void *context)
{
    if (pending) {
        debug_printf("[CMD] busy: wait for previous result\r\n");
        return false;
    }
    if (submit==NULL || (uint32_t)opcode>UINT16_MAX) {
        debug_printf("[CMD] invalid request\r\n");
        return false;
    }
    if (next_id==UINT32_MAX) {
        debug_printf("[CMD] request IDs exhausted; restart required\r\n");
        return false;
    }
    const uint32_t id=++next_id;
    if (!submit(APP_TASK_CMD,id,context)) {
        debug_printf("[CMD] request not queued\r\n");
        return false;
    }
    pending=true;
    pending_id=id;
    pending_opcode=opcode;
    debug_printf("[CMD] accepted id=0x%08" PRIX32 " op=0x%04X; awaiting result\r\n",
                 id,(unsigned)opcode);
    return true;
}

void CmdRequest_ProcessResult(void)
{
    static SharedSnapshot_t snapshot;
    if (!pending || !SharedData_GetSnapshot(&snapshot)) { return; }
    const CommandResultStatus_t *result=&snapshot.result[APP_TASK_CMD];
    if (!result->valid || result->message.header.request_id!=pending_id ||
        result->message.header.opcode!=pending_opcode) { return; }
    const char *state;
    switch (result->message.status) {
    case APP_RESULT_OK: state="done"; break;
    case APP_RESULT_STORED: state="stored; hardware not applied"; break;
    case APP_RESULT_TIMEOUT: state="timeout; execution may still complete"; break;
    case APP_RESULT_BUSY: state="worker busy or update unavailable"; break;
    case APP_RESULT_INVALID: state="invalid request"; break;
    case APP_RESULT_UNSUPPORTED: state="unsupported request"; break;
    default: state="failed"; break;
    }
    debug_printf("[CMD] result id=0x%08" PRIX32 " op=0x%04X status=%" PRId32 " %s\r\n",
                 pending_id,(unsigned)pending_opcode,result->message.status,state);
    if (result->message.requested_mask!=0U) {
        debug_printf("[CMD] lanes requested=0x%02X success=0x%02X failed=0x%02X\r\n",
                     (unsigned)result->message.requested_mask,(unsigned)result->message.success_mask,
                     (unsigned)result->message.failed_mask);
    }
    pending=false;
}
