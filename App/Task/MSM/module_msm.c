#include "module_msm.h"
#include <stddef.h>

void ModuleMsm_Init(ModuleMsm_t *msm, const MsmHooks_t *hooks, void *user)
{
    if (msm == NULL) {
        return;
    }
    *msm = (ModuleMsm_t){
        .state = MSM_RESETTING,
        .generation = 1U,
        .user = user
    };
    if (hooks != NULL) {
        msm->hooks = *hooks;
    }
    if (msm->hooks.enter != NULL) {
        msm->hooks.enter(user, msm->state, msm->generation);
    }
}

bool ModuleMsm_Step(ModuleMsm_t *msm, const MsmInputs_t *inputs)
{

    if ((msm == NULL) || (inputs == NULL)) {
        return false;
    }
    MsmState_t next = msm->state;
    const bool done = inputs->operation_done &&
                     (inputs->operation_generation == msm->generation);

    switch (msm->state) {
    case MSM_RESETTING:
        if (done) { next = MSM_RESET; }
        break;
    case MSM_RESET:
        if (!inputs->reset) { next = MSM_MGMT_INIT; }
        break;
    case MSM_MGMT_INIT:
        if (inputs->reset) { next = MSM_RESETTING; }
        else if (inputs->fault) { next = MSM_MODULE_FAULT; }
        else if (done) { next = MSM_MODULE_LOW_PWR; }
        break;
    case MSM_MODULE_LOW_PWR:
        if (inputs->reset) { next = MSM_RESETTING; }
        else if (inputs->fault) { next = MSM_MODULE_FAULT; }
        else if (!inputs->low_power) { next = MSM_MODULE_PWR_UP; }
        break;
    case MSM_MODULE_PWR_UP:
        if (inputs->reset) { next = MSM_RESETTING; }
        else if (inputs->fault) { next = MSM_MODULE_FAULT; }
        else if (inputs->low_power) { next = MSM_MODULE_PWR_DN; }
        else if (done) { next = MSM_MODULE_READY; }
        break;
    case MSM_MODULE_READY:
        if (inputs->reset) { next = MSM_RESETTING; }
        else if (inputs->fault) { next = MSM_MODULE_FAULT; }
        else if (inputs->low_power_exit) { next = MSM_MODULE_PWR_DN; }
        break;
    case MSM_MODULE_PWR_DN:
        if (inputs->reset) { next = MSM_RESETTING; }
        else if (inputs->fault) { next = MSM_MODULE_FAULT; }
        else if (done) { next = MSM_MODULE_LOW_PWR; }
        break;
    case MSM_MODULE_FAULT:
        /* Fault stays latched even if the input fault clears. */
        if (inputs->reset) { next = MSM_RESETTING; }
        break;
    default:
        next = MSM_MODULE_FAULT;
        break;
    }

    if (next == msm->state) {
        return false;
    }
    if (msm->hooks.exit != NULL) {
        msm->hooks.exit(msm->user, msm->state, msm->generation);
    }
    msm->state = next;
    ++msm->generation;
    if (msm->hooks.enter != NULL) {
        msm->hooks.enter(msm->user, msm->state, msm->generation);
    }
    return true;
}

const char *ModuleMsm_StateName(MsmState_t state)
{
    static const char *const names[MSM_STATE_COUNT] = {
        "Resetting", "Reset", "MgmtInit", "ModuleLowPwr",
        "ModulePwrUp", "ModuleReady", "ModulePwrDn", "ModuleFault"
    };
    return ((unsigned)state < MSM_STATE_COUNT) ? names[state] : "Invalid";
}
