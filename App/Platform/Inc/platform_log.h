#ifndef PLATFORM_LOG_H
#define PLATFORM_LOG_H

/* 1: enable debug output; 0: compile out output and argument evaluation.
 * Apply consistently to the whole project; rebuild after changing it. */
#ifndef PLATFORM_LOG_ENABLE
#define PLATFORM_LOG_ENABLE 1
#endif

#if (PLATFORM_LOG_ENABLE != 0) && (PLATFORM_LOG_ENABLE != 1)
#error "PLATFORM_LOG_ENABLE must be 0 or 1"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Call once during startup, after kernel initialization and before tasks run.
 * Returns 0 on success, -1 on failure. Disabled logging is a successful no-op. */
int PlatformLog_Init(void);

/* Synchronous task-only output. May block waiting for the mutex and UART.
 * Returns the formatted character count, or a negative value on failure.
 * Low-level BSP UART errors are not reported by the current stdout backend.
 * All concurrent stdout writers must use this API for serialization.
 * Do not call from interrupts or recursively from the output backend. */
#if defined(__GNUC__)
__attribute__((format(printf, 1, 2)))
#endif
int PlatformLog_Printf(const char *format, ...);

#if PLATFORM_LOG_ENABLE
#define debug_printf(...) PlatformLog_Printf(__VA_ARGS__)
#else
/* Unreachable branch preserves format checking and variable references,
 * but never evaluates arguments or calls the backend. Returns 0. */
static inline int PlatformLog_Disabled(void) { return 0; }
#define debug_printf(...) (0 ? PlatformLog_Printf(__VA_ARGS__) : PlatformLog_Disabled())
#endif

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_LOG_H */
