#ifndef PLATFORM_EVENT_H
#define PLATFORM_EVENT_H
#include <stdint.h>
typedef void *PlatformEventTarget_t;
typedef enum { PLATFORM_EVENT_OK=0, PLATFORM_EVENT_TIMEOUT,
               PLATFORM_EVENT_INVALID, PLATFORM_EVENT_ERROR } PlatformEventStatus_t;
#define PLATFORM_EVENT_WAIT_FOREVER UINT32_MAX
/* Thread flags, no separate event object/allocation. Bits 0..30 only.
 * Set is allowed from tasks or RTOS-compatible ISRs after target creation.
 * Never call RTOS APIs from higher urgency interrupts than the FreeRTOS limit.
 * Flags coalesce: use Queue/shared memory to hold actual data. */
PlatformEventStatus_t PlatformEvent_Set(PlatformEventTarget_t target, uint32_t flags);
/* Wait ANY on calling task, auto-clear matched flags. Task context only.
 * timeout is milliseconds (0=poll). received is zero on failure/timeout. */
PlatformEventStatus_t PlatformEvent_Wait(uint32_t mask, uint32_t timeout_ms,
                                        uint32_t *received);
#endif
