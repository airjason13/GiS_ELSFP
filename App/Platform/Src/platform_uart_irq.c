#include "platform_uart_irq.h"
#include "stm32h5xx_nucleo.h"

#define UART_RX_CAPACITY 512U
#define UART_RX_IRQ_PRIORITY 6U
static uint8_t rx_byte;
static uint8_t rx_buffer[UART_RX_CAPACITY];
static volatile uint32_t head, tail;
static volatile bool lost, restart;
static bool started;

bool PlatformUartIrq_Start(void)
{
    if (started) { return true; }
    if (hcom_uart[COM1].Instance != USART2) { return false; }
    HAL_NVIC_DisableIRQ(USART2_IRQn);
    head=0U; tail=0U; lost=false; restart=false;
    HAL_NVIC_SetPriority(USART2_IRQn, UART_RX_IRQ_PRIORITY, 0U);
    HAL_NVIC_ClearPendingIRQ(USART2_IRQn);
    if (HAL_UART_Receive_IT(&hcom_uart[COM1], &rx_byte, 1U) != HAL_OK) {
        return false;
    }
    started=true;
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    return true;
}

void PlatformUartIrq_Handler(void)
{
    HAL_UART_IRQHandler(&hcom_uart[COM1]);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uart)
{
    if (uart != &hcom_uart[COM1]) { return; }
    if (uart->ErrorCode != HAL_UART_ERROR_NONE) {
        lost=true;
        restart=true;
        return;
    }
    uint32_t next=(head+1U)%UART_RX_CAPACITY;
    if (next==tail) { lost=true; }
    else {
        rx_buffer[head]=rx_byte;
        __DMB();
        head=next;
    }
    if (HAL_UART_Receive_IT(uart, &rx_byte, 1U) != HAL_OK) {
        lost=true;
        restart=true;
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *uart)
{
    if (uart == &hcom_uart[COM1]) {
        lost=true;
        restart=true; /* Recover in task context, never log inside IRQ. */
    }
}

void PlatformUartIrq_Service(void)
{
    if (!started || !restart) { return; }
    HAL_NVIC_DisableIRQ(USART2_IRQn);
    if (HAL_UART_AbortReceive(&hcom_uart[COM1]) == HAL_OK) {
        __HAL_UART_CLEAR_FLAG(&hcom_uart[COM1], UART_CLEAR_PEF | UART_CLEAR_FEF |
                             UART_CLEAR_NEF | UART_CLEAR_OREF);
        __HAL_UART_SEND_REQ(&hcom_uart[COM1], UART_RXDATA_FLUSH_REQUEST);
        restart=HAL_UART_Receive_IT(&hcom_uart[COM1], &rx_byte, 1U)!=HAL_OK;
    }
    HAL_NVIC_EnableIRQ(USART2_IRQn);
}

int PlatformUartIrq_ReadByte(uint8_t *byte)
{
    if (byte==NULL || !started) { return 0; }
    const uint32_t primask=__get_PRIMASK();
    __disable_irq();
    int result=0;
    if (lost) {
        tail=head;
        lost=false;
        result=-1;
    } else if (tail!=head) {
        *byte=rx_buffer[tail];
        tail=(tail+1U)%UART_RX_CAPACITY;
        result=1;
    }
    __set_PRIMASK(primask);
    return result;
}
