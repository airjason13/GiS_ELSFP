#include "msm_port.h"

void MsmPort_ReadInputs(const ModuleMsm_t *msm, MsmInputs_t *inputs)
{
    (void)msm;
    /* TODO: snapshot ResetS/FaultS/LowPwrS/LowPwrExS from their owners.
     * TODO: match completion to the generation sent by MsmPort_Enter.
     * Do not set operation_done merely because a command was queued.
     * Defaults intentionally keep the unfinished framework in Resetting. */
    *inputs = (MsmInputs_t){ .low_power = true };
}

void MsmPort_Enter(void *user, MsmState_t state, uint32_t generation)
{
    (void)user;
    (void)generation;
    /* TODO: publish state using an explicit CMIS encoding map.
     * Dispatch nonblocking operations here, tagged with generation.
     * Actual hardware interlocks/protection must not depend on this poll. */
    switch (state) {
    case MSM_RESETTING:
        /* TODO: reset management and module electronics; report completion. */
        break;
    case MSM_RESET:
        /* TODO: maintain reset until ResetS is deasserted. */
        break;
    case MSM_MGMT_INIT:
        /* TODO: initialize I2C management and CMIS defaults; report completion. */
        break;
    case MSM_MODULE_LOW_PWR:
        /* TODO: maintain low power and enable management access. */
        break;
    case MSM_MODULE_PWR_UP:
        /* TODO: request Laser/TEC power-up; aggregate required completions. */
        break;
    case MSM_MODULE_READY:
        /* TODO: allow normal module commands. */
        break;
    case MSM_MODULE_PWR_DN:
        /* TODO: request power-down; confirm actual low power before done. */
        break;
    case MSM_MODULE_FAULT:
        /* TODO: latch fault, inhibit enable commands, request safe shutdown.
         * Remain here after shutdown until ResetS. This stub does not itself
         * disable hardware; SafetyTask/hardware must enforce protection. */
        break;
    default:
        break;
    }
}

void MsmPort_Exit(void *user, MsmState_t state, uint32_t generation)
{
    (void)user;
    (void)state;
    (void)generation;
    /* TODO: retire/cancel pending operations for this entry. Late results
     * must retain their old generation and cannot complete the next entry. */
}
