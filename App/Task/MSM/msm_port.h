#ifndef MSM_PORT_H
#define MSM_PORT_H
#include "module_msm.h"

/* Application/hardware integration, owned by MsmTask. */
void MsmPort_ReadInputs(const ModuleMsm_t *msm, MsmInputs_t *inputs);
void MsmPort_Enter(void *user, MsmState_t state, uint32_t generation);
void MsmPort_Exit(void *user, MsmState_t state, uint32_t generation);
#endif
