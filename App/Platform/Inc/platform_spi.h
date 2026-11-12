#ifndef PLATFORM_SPI_H
#define PLATFORM_SPI_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Startup-only platform binding. Call after CubeMX MX_SPI2_Init(), before
 * starting tasks. Hardware configuration remains owned by CubeMX.
 * Returns false if SPI2 has not been initialized or is not ready.
 * Does not configure CS, start transfers, or create RTOS objects. */
bool spi_init(void);

/* Ready-state query only; not a mutex or a reservation of the SPI bus. */
bool spi_is_ready(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_SPI_H */
