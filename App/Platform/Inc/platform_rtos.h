#ifndef PLATFORM_RTOS_H
#define PLATFORM_RTOS_H
#include <stdbool.h>
#include <stdint.h>
/* Opaque handle; implementation belongs to the current RTOS backend. */
typedef void *PlatformMutex_t;
PlatformMutex_t PlatformMutex_Create(const char *name);
bool PlatformMutex_Lock(PlatformMutex_t mutex, uint32_t timeout_ms);
bool PlatformMutex_Unlock(PlatformMutex_t mutex);
/* Task context only; handle must refer to a live task. Units: bytes.
 * Historical minimum unused stack, not instantaneous free space. */
bool PlatformThread_GetStackMinFree(void *thread, uint32_t *bytes);

/* Inspect in debugger after a stack overflow; pointers may be corrupted. */
typedef struct {
    void *task;
    const char *name;
    uint32_t detected;
} PlatformStackOverflowInfo_t;
extern volatile PlatformStackOverflowInfo_t platform_stack_overflow;
#endif
