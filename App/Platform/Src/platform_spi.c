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
