#include "system_cmd.h"
#include "global_def.h"
#include "platform_rtos.h"
#include "app_freertos.h"
#include <stdio.h>

/* Early warning only; tune using worst-case workload measurements. */
#define CMD_STACK_WARN_BYTES 256U

static const char *const task_names[APP_TASK_COUNT] = {
    [APP_TASK_HOST] = "HostMgmt",
    [APP_TASK_MSM] = "MSM",
    [APP_TASK_LASER] = "LaserAPC",
    [APP_TASK_TEC] = "ThermalTEC",
    [APP_TASK_DDM] = "SupervisionDDM",
    [APP_TASK_SAFETY] = "Safety",
    [APP_TASK_STORAGE] = "Storage",
    [APP_TASK_CMD] = "Cmd"
};

static osThreadId_t const *const task_handles[APP_TASK_COUNT] = {
    [APP_TASK_HOST] = &HostMgmtTaskHandle,
    [APP_TASK_MSM] = &MsmTaskHandle,
    [APP_TASK_LASER] = &LaserApcTaskHandle,
    [APP_TASK_TEC] = &ThermalTecTaskHandle,
    [APP_TASK_DDM] = &SupervisionDDMTaskHandle,
    [APP_TASK_SAFETY] = &SafetyTaskHandle,
    [APP_TASK_STORAGE] = &StorageTaskHandle,
    [APP_TASK_CMD] = &CmdHandleTaskHandle
};


void SystemCmd_Status(void)
{
    static SharedSnapshot_t snapshot;
    if (!SharedData_GetSnapshot(&snapshot)) {
        debug_printf("[CMD] shared data unavailable\r\n");
        return;
    }
    const uint32_t now = osKernelGetTickCount();
    static char stack_text[11];
    debug_printf("\r\n=== ELSFP Monitor | Now %" PRIu32 "  ===\r\n", now);
    debug_printf("%-14s | %10s | %10s | %3s | %8s | %10s | %10s | %s\r\n",
                 "Task", "Loops", "Events(hex)", "MSM", "Failures",
                 "Age(tick)", "StackMin B", "Stack");
    debug_printf("---------------+------------+------------+-----+----------+------------+------------+------\r\n");
    for (uint32_t i = 0U; i < APP_TASK_COUNT; ++i) {
        const TaskSharedStatus_t *status = &snapshot.task[i];
        uint32_t stack_min_free;
        const char *stack_state = "N/A";
        if (PlatformThread_GetStackMinFree(*task_handles[i], &stack_min_free)) {
            (void)snprintf(stack_text, sizeof(stack_text), "%" PRIu32, stack_min_free);
            stack_state = stack_min_free <= CMD_STACK_WARN_BYTES ? "LOW!" : "--";
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
                 (unsigned)CMD_STACK_WARN_BYTES);
    debug_printf("=========================================================================================\r\n");
}

