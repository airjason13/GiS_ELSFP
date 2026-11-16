#ifdef DEBUG_FLOW_TEST_ENABLE
#include "_debug_flow_test.h"
#endif

#include "debug_task.h"
#include "command_parser.h"
#include "global_def.h"
#include "platform_rtos.h"
#include "platform_uart_irq.h"
#include "app_freertos.h"

/* Early warning only; tune using worst-case workload measurements. */
#define DEBUG_STACK_WARN_BYTES 256U

#if DEBUG_SYS_INFO
static const char *const task_names[APP_TASK_COUNT] = {
    [APP_TASK_HOST] = "HostMgmt",
    [APP_TASK_MSM] = "MSM",
    [APP_TASK_LASER] = "LaserAPC",
    [APP_TASK_TEC] = "ThermalTEC",
    [APP_TASK_DDM] = "SupervisionDDM",
    [APP_TASK_SAFETY] = "Safety",
    [APP_TASK_STORAGE] = "Storage",
    [APP_TASK_DEBUG] = "Debug"
};

static osThreadId_t const *const task_handles[APP_TASK_COUNT] = {
    [APP_TASK_HOST] = &HostMgmtTaskHandle,
    [APP_TASK_MSM] = &MsmTaskHandle,
    [APP_TASK_LASER] = &LaserApcTaskHandle,
    [APP_TASK_TEC] = &ThermalTecTaskHandle,
    [APP_TASK_DDM] = &SupervisionDDMTaskHandle,
    [APP_TASK_SAFETY] = &SafetyTaskHandle,
    [APP_TASK_STORAGE] = &StorageTaskHandle,
    [APP_TASK_DEBUG] = &SysdebugTaskHandle
};

#endif

static void Debug_SystemStatus(const SharedSnapshot_t *snapshot, uint32_t now)
{
#if DEBUG_SYS_INFO
    static char stack_text[11];
    debug_printf("\r\n=== ELSFP Monitor | Startup %" PRIu32 "(Ms)  ===\r\n", now);
    debug_printf("%-14s | %10s | %10s | %3s | %8s | %10s | %10s | %s\r\n",
                 "Task", "Loops", "Events(hex)", "MSM", "Failures",
                 "Age(tick)", "StackMin B", "Stack");
    debug_printf("---------------+------------+------------+-----+----------+------------+------------+------\r\n");
    for (uint32_t i = 0U; i < APP_TASK_COUNT; ++i) {
        const TaskSharedStatus_t *status = &snapshot->task[i];
        uint32_t stack_min_free;
        const char *stack_state = "N/A";
        if (PlatformThread_GetStackMinFree(*task_handles[i], &stack_min_free)) {
            (void)snprintf(stack_text, sizeof(stack_text), "%" PRIu32, stack_min_free);
            stack_state = stack_min_free <= DEBUG_STACK_WARN_BYTES ? "LOW!" : "--";
        } else {
            stack_text[0] = '-';
            stack_text[1] = '\0';
        }
        if (!status->valid) {
            debug_printf("%-14s | %10s | %10s | %3s | %8s | %10s | %10s | %s (unpublished)\r\n",
                         task_names[i], "-", "-", "-", "-", "-", stack_text, stack_state);
            continue;
        }
        debug_printf("%-14s | %10" PRIu32 " | 0x%08" PRIX32
                     " | %3" PRIu32 " | %8" PRIu32 " | %10" PRIu32 " | %10s | %s\r\n",
                     task_names[i], status->iterations, status->last_events,
                     status->observed_module_state, status->failures,
                     (uint32_t)(now - status->sample_tick), stack_text, stack_state);
    }
    debug_printf("StackMin = minimum free bytes; LOW! <= %u bytes; MSM = observed state ID\r\n",
                 (unsigned)DEBUG_STACK_WARN_BYTES);
    debug_printf("=========================================================================================\r\n");
#else
    (void)snapshot;
    (void)now;
#endif
}

#if DEBUG_UART_RX_ENABLE
static void Debug_UartReceive(void)
{
    static char line[COMMAND_LINE_MAX + 1U]; /* 64 payload bytes plus NUL. */
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
            debug_printf("[UART RX] data lost; discard until newline\r\n");
            continue;
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
#endif

void DebugTask_Run(void *argument)
{
    (void)argument;
    static AppTaskRuntime_t ctx;
#if DEBUG_UART_RX_ENABLE
    const bool uart_ready=PlatformUartIrq_Start();
    //debug_printf("[UART RX] %s\r\n",uart_ready ? "USART2 ready; type text then Enter" : "start failed");
#endif
    uint32_t last_print = osKernelGetTickCount();
    bool first = true;
    for (;;) {
#if DEBUG_UART_RX_ENABLE
        if (uart_ready) { Debug_UartReceive(); }
#endif
        const bool have_snapshot = AppTask_ReadSnapshot(&ctx);
        const uint32_t now = osKernelGetTickCount();
        if (have_snapshot) {
			#ifdef DEBUG_FLOW_TEST_ENABLE
        	DebugFlowTest_Poll(&ctx.snapshot, now);
			#endif
        }
        if (first || (uint32_t)(now - last_print) >= (10U * osKernelGetTickFreq())) {
            first = false;
            last_print = now;
            if (have_snapshot) {
                Debug_SystemStatus(&ctx.snapshot, now);
            } else {
                debug_printf("[DEBUG_TASK] snapshot unavailable\r\n");
            }
        }
        AppTask_PublishStatus(&ctx, APP_TASK_DEBUG);
        /* Event wakeups do not change the periodic system-info interval. */
        #if DEBUG_UART_RX_ENABLE
        AppTask_WaitEvents(&ctx, APP_EVT_MODULE, 10U); /* Responsive RX echo. */
#else
        AppTask_WaitEvents(&ctx, APP_EVT_MODULE, 100U);
#endif
    }
}
