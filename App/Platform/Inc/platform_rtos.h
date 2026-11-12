#ifndef PLATFORM_RTOS_H
#define PLATFORM_RTOS_H
#include <stdbool.h>
#include <stdint.h>
/* Opaque handle; implementation belongs to the current RTOS backend. */
typedef void *PlatformMutex_t;
PlatformMutex_t PlatformMutex_Create(const char *name);
bool PlatformMutex_Lock(PlatformMutex_t mutex, uint32_t timeout_ms);
bool PlatformMutex_Unlock(PlatformMutex_t mutex);
#endif
