#include "platform_spi.h"
#include "main.h"
#include <stddef.h>

/* STM32-specific binding stays in this implementation, not the public API.
 * hspi2 is owned and initialized by CubeMX-generated main.c. */
extern SPI_HandleTypeDef hspi2;
static SPI_HandleTypeDef *spi_handle;

bool spi_init(void)
{
    if ((hspi2.Instance != SPI2) ||
        (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY)) {
        return false;
    }

    spi_handle = &hspi2;
    return true;
}

bool spi_is_ready(void)
{
    return (spi_handle != NULL) &&
           (HAL_SPI_GetState(spi_handle) == HAL_SPI_STATE_READY);
}

static SpiStatus_t spi_check_transfer(const void *data, size_t length,
                                      uint32_t timeout_ms)
{
    if ((data == NULL) || (length == 0U) || (length >= UINT16_MAX) ||
        (timeout_ms == UINT32_MAX)) {
        return SPI_STATUS_INVALID_ARG;
    }
    if (spi_handle == NULL) {
        return SPI_STATUS_NOT_INITIALIZED;
    }
    if ((spi_handle->Init.DataSize != SPI_DATASIZE_8BIT) ||
        (spi_handle->Init.Mode != SPI_MODE_MASTER) ||
        (spi_handle->Init.Direction != SPI_DIRECTION_2LINES)) {
        return SPI_STATUS_ERROR;
    }
    return SPI_STATUS_OK;
}

static SpiStatus_t spi_convert_status(HAL_StatusTypeDef status)
{
    switch (status) {
    case HAL_OK:      return SPI_STATUS_OK;
    case HAL_BUSY:    return SPI_STATUS_BUSY;
    case HAL_TIMEOUT: return SPI_STATUS_TIMEOUT;
    default:         return SPI_STATUS_ERROR;
    }
}

SpiStatus_t spi_write(const uint8_t *data, size_t length, uint32_t timeout_ms)
{
    SpiStatus_t status = spi_check_transfer(data, length, timeout_ms);
    if (status != SPI_STATUS_OK) {
        return status;
    }
    return spi_convert_status(HAL_SPI_Transmit(spi_handle, data,
                                             (uint16_t)length, timeout_ms));
}

SpiStatus_t spi_read(uint8_t *data, size_t length, uint32_t timeout_ms)
{
    SpiStatus_t status = spi_check_transfer(data, length, timeout_ms);
    if (status != SPI_STATUS_OK) {
        return status;
    }
    return spi_convert_status(HAL_SPI_Receive(spi_handle, data,
                                            (uint16_t)length, timeout_ms));
}
