#include "platform_event.h"
#include "cmsis_os2.h"
#include <stddef.h>
static int valid_flags(uint32_t flags)
{ return flags != 0U && (flags & 0x80000000U) == 0U; }
PlatformEventStatus_t PlatformEvent_Set(PlatformEventTarget_t target, uint32_t flags)
{
    if (target == NULL || !valid_flags(flags)) { return PLATFORM_EVENT_INVALID; }
    uint32_t result = osThreadFlagsSet(target, flags);
    return (result & osFlagsError) ? PLATFORM_EVENT_ERROR : PLATFORM_EVENT_OK;
}
PlatformEventStatus_t PlatformEvent_Wait(uint32_t mask, uint32_t timeout_ms,
                                        uint32_t *received)
{
    if (received == NULL) { return PLATFORM_EVENT_INVALID; }
    *received = 0U;
    if (!valid_flags(mask)) { return PLATFORM_EVENT_INVALID; }
    uint32_t timeout;
    if (timeout_ms == PLATFORM_EVENT_WAIT_FOREVER) { timeout = osWaitForever; }
    else {
        uint64_t ticks = ((uint64_t)timeout_ms * osKernelGetTickFreq() + 999U) / 1000U;
        timeout = ticks >= UINT32_MAX ? UINT32_MAX - 1U : (uint32_t)ticks;
    }
    uint32_t result = osThreadFlagsWait(mask, osFlagsWaitAny, timeout);
    if (result == osFlagsErrorTimeout || result == osFlagsErrorResource) {
        return PLATFORM_EVENT_TIMEOUT;
    }
    if ((result & osFlagsError) != 0U) { return PLATFORM_EVENT_ERROR; }
    *received = result & mask;
    return PLATFORM_EVENT_OK;
}
