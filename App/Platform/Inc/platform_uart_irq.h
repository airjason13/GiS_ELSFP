#ifndef PLATFORM_UART_IRQ_H
#define PLATFORM_UART_IRQ_H
#include <stdbool.h>
#include <stdint.h>
/* USART2 / BSP COM1. Call once from Debug Task after BSP_COM_Init.
 * Does not reinitialize UART or change existing printf TX configuration. */
bool PlatformUartIrq_Start(void);
/* Debug Task only: recover a failed/error RX transfer (TX remains untouched). */
void PlatformUartIrq_Service(void);
/* Single task consumer: 1=byte, 0=empty, -1=data lost, discard current line.
 * On loss buffered bytes are flushed; resynchronize at the next newline. */
int PlatformUartIrq_ReadByte(uint8_t *byte);
/* Called only by USART2_IRQHandler. */
void PlatformUartIrq_Handler(void);
#endif
