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

/* FreeRTOS-specific diagnostics stay in the platform backend. */
#include "FreeRTOS.h"
#include "task.h"
#include "stm32h5xx.h"

volatile PlatformStackOverflowInfo_t platform_stack_overflow;

bool PlatformThread_GetStackMinFree(void *thread, uint32_t *bytes)
{
    if (bytes == NULL) { return false; }
    *bytes = 0U;
    if (thread == NULL || __get_IPSR() != 0U ||
        osKernelGetState() != osKernelRunning) { return false; }
    *bytes = osThreadGetStackSpace((osThreadId_t)thread);
    return true;
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    /* Do not printf, allocate, lock a mutex or attempt recovery here. */
    __disable_irq();
    platform_stack_overflow.task = task;
    platform_stack_overflow.name = name;
    platform_stack_overflow.detected = 1U;
    if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0U) {
        __BKPT(0);
    }
    for (;;) { __NOP(); }
}
