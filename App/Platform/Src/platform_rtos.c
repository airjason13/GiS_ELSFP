#include "platform_rtos.h"
#include "cmsis_os2.h"
#include <stddef.h>
PlatformMutex_t PlatformMutex_Create(const char *name)
{
    const osMutexAttr_t attr = {.name=name, .attr_bits=osMutexPrioInherit};
    return osMutexNew(&attr);
}
bool PlatformMutex_Lock(PlatformMutex_t mutex, uint32_t timeout_ms)
{
    if (mutex == NULL || osKernelGetState() != osKernelRunning) { return false; }
    uint64_t ticks = ((uint64_t)timeout_ms * osKernelGetTickFreq() + 999U) / 1000U;
    /* Keep a finite timeout; do not accidentally convert to osWaitForever. */
    if (ticks >= UINT32_MAX) { ticks = UINT32_MAX - 1U; }
    return osMutexAcquire(mutex, (uint32_t)ticks) == osOK;
}
bool PlatformMutex_Unlock(PlatformMutex_t mutex)
{
    return mutex != NULL && osMutexRelease(mutex) == osOK;
}
