#include "msm_task.h"
#include "global_def.h"
#include "module_msm.h"
#include "msm_port.h"

/* Set to 0 to disable MSM diagnostics without changing state-machine logic. */
#ifndef MSM_DEBUG_ENABLE
#define MSM_DEBUG_ENABLE 1
#endif

void MsmTask_Run(void *argument)
{
    (void)argument;
    ModuleMsm_t msm;
    const MsmHooks_t hooks = {
        .enter = MsmPort_Enter,
        .exit = MsmPort_Exit
    };
    /* 10 ms polling scaffold; hardware protection is independent. */
    const uint32_t period = osKernelGetTickFreq() / 100U +
                           ((osKernelGetTickFreq() % 100U) != 0U);

    ModuleMsm_Init(&msm, &hooks, NULL);
#if MSM_DEBUG_ENABLE
    uint32_t last_status_tick = osKernelGetTickCount();
    const uint32_t status_interval = osKernelGetTickFreq() * 5U;
    debug_printf("[MSM] start state=%s gen=%" PRIu32 " poll=%" PRIu32 " ticks\r\n",
                 ModuleMsm_StateName(msm.state), msm.generation, period);
    debug_printf("[MSM] hardware hooks pending; waiting for real completion\r\n");
#endif
    for (;;) {
        MsmInputs_t inputs;
        MsmPort_ReadInputs(&msm, &inputs);
#if MSM_DEBUG_ENABLE
        const MsmState_t previous_state = msm.state;
#endif
        const bool changed = ModuleMsm_Step(&msm, &inputs);
#if MSM_DEBUG_ENABLE
        if (changed) {
            debug_printf("[MSM] %s -> %s gen=%" PRIu32
                         " reset=%u fault=%u low=%u lowEx=%u done=%u doneGen=%" PRIu32 "\r\n",
                         ModuleMsm_StateName(previous_state),
                         ModuleMsm_StateName(msm.state), msm.generation,
                         (unsigned)inputs.reset, (unsigned)inputs.fault,
                         (unsigned)inputs.low_power, (unsigned)inputs.low_power_exit,
                         (unsigned)inputs.operation_done, inputs.operation_generation);
        }
        const uint32_t now = osKernelGetTickCount();
        /* Unsigned subtraction also handles the RTOS tick counter wrapping. */
        if ((uint32_t)(now - last_status_tick) >= status_interval) {
            last_status_tick = now;
            debug_printf("[MSM] status state=%s gen=%" PRIu32
                         " reset=%u fault=%u low=%u lowEx=%u done=%u doneGen=%" PRIu32 "\r\n",
                         ModuleMsm_StateName(msm.state), msm.generation,
                         (unsigned)inputs.reset, (unsigned)inputs.fault,
                         (unsigned)inputs.low_power, (unsigned)inputs.low_power_exit,
                         (unsigned)inputs.operation_done, inputs.operation_generation);
        }
#else
        (void)changed;
#endif
        osDelay(period);
    }
}
