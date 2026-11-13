#ifndef PLATFORM_SPI_H
#define PLATFORM_SPI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Startup-only platform binding. Call after CubeMX MX_SPI2_Init(), before
 * starting tasks. Hardware configuration remains owned by CubeMX.
 * Returns false if SPI2 has not been initialized or is not ready.
 * Does not configure CS, start transfers, or create RTOS objects. */
bool spi_init(void);

typedef enum {
    SPI_STATUS_OK = 0,
    SPI_STATUS_INVALID_ARG,
    SPI_STATUS_NOT_INITIALIZED,
    SPI_STATUS_BUSY,
    SPI_STATUS_TIMEOUT,
    SPI_STATUS_ERROR
} SpiStatus_t;

/* Blocking SPI2 transfers, length in bytes (1..65534), timeout in ms.
 * UINT32_MAX is rejected to keep waits finite. Requires 8-bit master mode.
 * Task/startup context only, not ISR. No CS or bus mutex is managed here:
 * caller must serialize the entire CS-low / write / read / CS-high transaction.
 * Write discards received data. Read clocks input using HAL receive mode;
 * it does not specify a MOSI dummy-byte pattern or send a register address.
 * Failure may leave a partially transferred buffer; no automatic retry. */
SpiStatus_t spi_write(const uint8_t *data, size_t length, uint32_t timeout_ms);
SpiStatus_t spi_read(uint8_t *data, size_t length, uint32_t timeout_ms);

/* Ready-state query only; not a mutex or a reservation of the SPI bus. */
bool spi_is_ready(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_SPI_H */
