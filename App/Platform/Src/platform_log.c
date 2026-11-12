#include "platform_log.h"
#if PLATFORM_LOG_ENABLE
#include "cmsis_os2.h"
#include <stdarg.h>
#include <stdio.h>

static osMutexId_t log_mutex;

int PlatformLog_Init(void)
{
    static const osMutexAttr_t attributes = {
        .name = "PlatformLog",
        .attr_bits = osMutexPrioInherit
    };

    /* Startup only: after osKernelInitialize, before osKernelStart. */
    if (osKernelGetState() != osKernelReady) {
        return -1;
    }
    if (log_mutex == NULL) {
        log_mutex = osMutexNew(&attributes);
    }
    return (log_mutex != NULL) ? 0 : -1;
}

int PlatformLog_Printf(const char *format, ...)
{
    va_list args;
    int result;

    if ((format == NULL) || (log_mutex == NULL) ||
        (osKernelGetState() != osKernelRunning)) {
        return -1;
    }

    /* CMSIS rejects acquisition from ISR context. No output on failure. */
    if (osMutexAcquire(log_mutex, osWaitForever) != osOK) {
        return -1;
    }

    va_start(args, format);
    result = vprintf(format, args);
    va_end(args);

    /* Keep buffered output within the same protected operation. */
    if (fflush(stdout) == EOF) {
        result = -1;
    }
    if (osMutexRelease(log_mutex) != osOK) {
        result = -1;
    }
    return result;
}

#else
int PlatformLog_Init(void)
{
    return 0; /* No logging mutex is needed when output is disabled. */
}
#endif /* PLATFORM_LOG_ENABLE */
