#include "cmd_task.h"
#include "command_parser.h"
#include "cmd_request.h"
#include "global_def.h"
#include "platform_rtos.h"
#include "platform_uart_irq.h"
#include "app_freertos.h"
#include <string.h>

static void UartReceive(void)
{
    static char line[COMMAND_LINE_MAX + 1U]; /* 64 payload bytes plus NUL. */
    static char history[COMMAND_LINE_MAX + 1U]; /* Last submitted line only. */
    static uint8_t escape; /* 0: text, 1: ESC, 2: CSI/SS3, 3: parameters. */
    static unsigned length;
    static bool discard;
    static bool previous_cr;
    PlatformUartIrq_Service();
    /* Bounded work so a continuous sender cannot monopolize the task. */
    for (unsigned i=0U; i<COMMAND_LINE_MAX; ++i) {
        uint8_t byte;
        int result=PlatformUartIrq_ReadByte(&byte);
        if (result==0) { break; }
        if (result<0) {
            length=0U;
            discard=true;
            previous_cr=false;
            escape=0U;
            debug_printf("[UART RX] data lost; discard until newline\r\n");
            continue;
        }
        /* Arrow keys arrive as ESC [ A (or ESC O A), possibly across polls.
         * Consume other terminal key sequences without inserting their bytes. */
        if (byte=='\r' || byte=='\n') {
            escape=0U;
        } else if (!discard) {
            if (byte==27U) { escape=1U; continue; }
            if (escape!=0U) {
                if (escape==1U) {
                    escape=(byte=='[' || byte=='O') ? 2U : 0U;
                    continue;
                }
                if (byte>=0x40U && byte<=0x7eU) {
                    if (escape==2U && byte=='A' && history[0]!='\0') {
                        while (length!=0U) {
                            --length;
                            (void)send_char('\b');
                            (void)send_char(' ');
                            (void)send_char('\b');
                        }
                        length=(unsigned)strlen(history);
                        memcpy(line,history,length+1U);
                        for (unsigned j=0U; j<length; ++j) {
                            (void)send_char(line[j]);
                        }
                    }
                    escape=0U;
                } else if (byte>=0x20U && byte<=0x3fU) {
                    escape=3U;
                } else { escape=0U; }
                continue;
            }
        }
        if (byte=='\n' && previous_cr) {
            previous_cr=false;
            continue; /* CRLF is one line ending. */
        }
        previous_cr=(byte=='\r');
        if (byte=='\r' || byte=='\n') {
            (void)send_char('\r');
            (void)send_char('\n');
            if (!discard && length!=0U) {
                line[length]='\0';
                /* Empty/space-only input must not replace useful history. */
                if (strspn(line," ")!=length) {
                    memcpy(history,line,length+1U);
                }
                CommandParser_Parse(line);
            }
            length=0U;
            discard=false;
        } else if (!discard) {
            if (byte==8U || byte==127U) {
                if (length!=0U) {
                    --length;
                    (void)send_char('\b');
                    (void)send_char(' ');
                    (void)send_char('\b');
                }
            } else if (byte>=32U && byte<=126U) {
                if (length<sizeof(line)-1U) {
                    line[length++]=(char)byte;
                    (void)send_char((char)byte);
                }
                else {
                    length=0U;
                    discard=true;
                    //debug_printf("[UART RX] line too long (max 64); discarded\r\n");
                }
            } else {
                length=0U;
                discard=true;
                //debug_printf("[UART RX] unsupported character; line discarded\r\n");
            }
        }
    }
}


void CmdTask_Run(void *argument)
{
    (void)argument;
    static AppTaskRuntime_t ctx;
    const bool uart_ready=PlatformUartIrq_Start();
    debug_printf("[UART RX] %s\r\n",uart_ready ? "USART ready; type text then Enter" : "start failed");
    for (;;) {
        CmdRequest_ProcessResult();
        if (uart_ready) { UartReceive(); }
        AppTask_PublishStatus(&ctx, APP_TASK_CMD);
        /* Poll UART promptly; system status is printed only by command. */
        AppTask_WaitEvents(&ctx, APP_EVT_MODULE, 10U);
    }
}
