#ifndef MODULE_MSM_H
#define MODULE_MSM_H

#include <stdbool.h>
#include <stdint.h>

/* Internal values, NOT CMIS register encodings. */
typedef enum {
    MSM_RESETTING = 0,
    MSM_RESET,
    MSM_MGMT_INIT,
    MSM_MODULE_LOW_PWR,
    MSM_MODULE_PWR_UP,
    MSM_MODULE_READY,
    MSM_MODULE_PWR_DN,
    MSM_MODULE_FAULT,
    MSM_STATE_COUNT
} MsmState_t;

typedef struct {
    bool reset;
    bool fault;
    bool low_power;
    bool low_power_exit; /* LowPwrExS: low-power request while ModuleReady. */
    bool operation_done;
    /* Completion must belong to the current state entry, not an old command. */
    uint32_t operation_generation;
} MsmInputs_t;

typedef struct {
    void (*enter)(void *user, MsmState_t state, uint32_t generation);
    void (*exit)(void *user, MsmState_t state, uint32_t generation);
} MsmHooks_t;

typedef struct {
    MsmState_t state;
    uint32_t generation;
    MsmHooks_t hooks;
    void *user;
} ModuleMsm_t;

/* Single owner: call only from MsmTask. Hooks must not block/reenter.
 * Initial state is Resetting. One transition at most per Step call.
 * Generation associates asynchronous completion with a particular entry. */
void ModuleMsm_Init(ModuleMsm_t *msm, const MsmHooks_t *hooks, void *user);
bool ModuleMsm_Step(ModuleMsm_t *msm, const MsmInputs_t *inputs);
const char *ModuleMsm_StateName(MsmState_t state);

#endif
