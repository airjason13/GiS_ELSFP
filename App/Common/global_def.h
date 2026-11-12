#ifndef APP_GLOBAL_DEF_H
#define APP_GLOBAL_DEF_H

/* Common C types and utilities used by application tasks. */
#include <stdint.h>     /* uint8_t, uint16_t, uint32_t, ... */
#include <stdbool.h>    /* bool, true, false */
#include <stddef.h>     /* size_t, NULL, offsetof */
#include <stdio.h>      /* printf, snprintf */
#include <stdlib.h>     /* strtol, abs, ... */
#include <string.h>     /* memcpy, memset, strcmp, ... */
#include <inttypes.h>   /* PRIu32 and other fixed-width format macros */
#include <limits.h>     /* Integer limits */

/* Current RTOS interface and application message definitions.
 * Keep STM32 HAL and peripheral handles in the Platform implementation. */
#include "cmsis_os2.h"
#include "messages_queue.h"
#include "shared_data.h"
#include "shared_data_service.h"
#include "platform_log.h"

#endif /* APP_GLOBAL_DEF_H */
