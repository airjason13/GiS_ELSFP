/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "platform_log.h"
#include "shared_data.h"
#include "messages_queue.h"
#include "debug_task.h"
#include "safety_task.h"
#include "host_task.h"
#include "msm_task.h"
#include "laser_task.h"
#include "thermal_task.h"
#include "supervision_task.h"
#include "storage_task.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for SysdebugTask */
osThreadId_t SysdebugTaskHandle;
const osThreadAttr_t SysdebugTask_attributes = {
  .name = "SysdebugTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 256 * 4
};
/* Definitions for SafetyTask */
osThreadId_t SafetyTaskHandle;
const osThreadAttr_t SafetyTask_attributes = {
  .name = "SafetyTask",
  .priority = (osPriority_t) osPriorityRealtime,
  .stack_size = 256 * 4
};
/* Definitions for HostMgmtTask */
osThreadId_t HostMgmtTaskHandle;
const osThreadAttr_t HostMgmtTask_attributes = {
  .name = "HostMgmtTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 512 * 4
};
/* Definitions for LaserApcTask */
osThreadId_t LaserApcTaskHandle;
const osThreadAttr_t LaserApcTask_attributes = {
  .name = "LaserApcTask",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 256 * 4
};
/* Definitions for ThermalTecTask */
osThreadId_t ThermalTecTaskHandle;
const osThreadAttr_t ThermalTecTask_attributes = {
  .name = "ThermalTecTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for SupervisionDDMTask */
osThreadId_t SupervisionDDMTaskHandle;
const osThreadAttr_t SupervisionDDMTask_attributes = {
  .name = "SupervisionDDMTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for StorageTask */
osThreadId_t StorageTaskHandle;
const osThreadAttr_t StorageTask_attributes = {
  .name = "StorageTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};
/* Definitions for MsmTask */
osThreadId_t MsmTaskHandle;
const osThreadAttr_t MsmTask_attributes = {
  .name = "MsmTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 256 * 4
};
/* Definitions for HostCmdMsgQueue */
osMessageQueueId_t HostCmdMsgQueueHandle;
const osMessageQueueAttr_t HostCmdMsgQueue_attributes = {
  .name = "HostCmdMsgQueue"
};
/* Definitions for LaserMsgQueue */
osMessageQueueId_t LaserMsgQueueHandle;
const osMessageQueueAttr_t LaserMsgQueue_attributes = {
  .name = "LaserMsgQueue"
};
/* Definitions for TecMsgQueue */
osMessageQueueId_t TecMsgQueueHandle;
const osMessageQueueAttr_t TecMsgQueue_attributes = {
  .name = "TecMsgQueue"
};
/* Definitions for ControlResultMsg */
osMessageQueueId_t ControlResultMsgHandle;
const osMessageQueueAttr_t ControlResultMsg_attributes = {
  .name = "ControlResultMsg"
};
/* Definitions for StorageCmdMsgQueue */
osMessageQueueId_t StorageCmdMsgQueueHandle;
const osMessageQueueAttr_t StorageCmdMsgQueue_attributes = {
  .name = "StorageCmdMsgQueue"
};
/* Definitions for MsmMsgQueue */
osMessageQueueId_t MsmMsgQueueHandle;
const osMessageQueueAttr_t MsmMsgQueue_attributes = {
  .name = "MsmMsgQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  if (PlatformLog_Init() != 0)
  {
    Error_Handler();
  }
  if (!SharedData_Init())
  {
    Error_Handler();
  }
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */
  /* creation of HostCmdMsgQueue */
  HostCmdMsgQueueHandle = osMessageQueueNew (32, sizeof(HostCmdMsg_t), &HostCmdMsgQueue_attributes);
  /* creation of LaserMsgQueue */
  LaserMsgQueueHandle = osMessageQueueNew (16, sizeof(LaserMsg_t), &LaserMsgQueue_attributes);
  /* creation of TecMsgQueue */
  TecMsgQueueHandle = osMessageQueueNew (16, sizeof(TecMsg_t), &TecMsgQueue_attributes);
  /* creation of ControlResultMsg */
  ControlResultMsgHandle = osMessageQueueNew (16, sizeof(ControlResultMsg_t), &ControlResultMsg_attributes);
  /* creation of StorageCmdMsgQueue */
  StorageCmdMsgQueueHandle = osMessageQueueNew (32, sizeof(StorageCmdMsg_t), &StorageCmdMsgQueue_attributes);
  /* creation of MsmMsgQueue */
  MsmMsgQueueHandle = osMessageQueueNew (16, sizeof(MsmMsg_t), &MsmMsgQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of SysdebugTask */
  SysdebugTaskHandle = osThreadNew(StartSysdebugTask, NULL, &SysdebugTask_attributes);

  /* creation of SafetyTask */
  SafetyTaskHandle = osThreadNew(StartSafetyTask, NULL, &SafetyTask_attributes);

  /* creation of HostMgmtTask */
  HostMgmtTaskHandle = osThreadNew(StartHostMgmtTask, NULL, &HostMgmtTask_attributes);

  /* creation of LaserApcTask */
  LaserApcTaskHandle = osThreadNew(StartLaserApcTask, NULL, &LaserApcTask_attributes);

  /* creation of ThermalTecTask */
  ThermalTecTaskHandle = osThreadNew(StartThermalTecTask, NULL, &ThermalTecTask_attributes);

  /* creation of SupervisionDDMTask */
  SupervisionDDMTaskHandle = osThreadNew(StartSupervisionDDMTask, NULL, &SupervisionDDMTask_attributes);

  /* creation of StorageTask */
  StorageTaskHandle = osThreadNew(StorageTask , NULL, &StorageTask_attributes);

  /* creation of MsmTask */
  MsmTaskHandle = osThreadNew(StartTask08, NULL, &MsmTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  if ((MsmTaskHandle == NULL) || (MsmMsgQueueHandle == NULL) ||
      (HostMgmtTaskHandle == NULL) || (LaserApcTaskHandle == NULL) ||
      (ThermalTecTaskHandle == NULL) || (SupervisionDDMTaskHandle == NULL) ||
      (SafetyTaskHandle == NULL) || (StorageTaskHandle == NULL) ||
      (SysdebugTaskHandle == NULL))
  {
    Error_Handler();
  }
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartSysdebugTask */
/**
* @brief Function implementing the SysdebugTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSysdebugTask */
void StartSysdebugTask(void *argument)
{
  /* USER CODE BEGIN SysdebugTask */
  DebugTask_Run(argument);
  /* USER CODE END SysdebugTask */
}

/* USER CODE BEGIN Header_StartSafetyTask */
/**
* @brief Function implementing the SafetyTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSafetyTask */
void StartSafetyTask(void *argument)
{
  /* USER CODE BEGIN SafetyTask */
  SafetyTask_Run(argument);
  /* USER CODE END SafetyTask */
}

/* USER CODE BEGIN Header_StartHostMgmtTask */
/**
* @brief Function implementing the HostMgmtTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartHostMgmtTask */
void StartHostMgmtTask(void *argument)
{
  /* USER CODE BEGIN HostMgmtTask */
  HostMgmtTask_Run(argument);
  /* USER CODE END HostMgmtTask */
}

/* USER CODE BEGIN Header_StartLaserApcTask */
/**
* @brief Function implementing the LaserApcTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLaserApcTask */
void StartLaserApcTask(void *argument)
{
  /* USER CODE BEGIN LaserApcTask */
  LaserTask_Run(argument);
  /* USER CODE END LaserApcTask */
}

/* USER CODE BEGIN Header_StartThermalTecTask */
/**
* @brief Function implementing the ThermalTecTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartThermalTecTask */
void StartThermalTecTask(void *argument)
{
  /* USER CODE BEGIN ThermalTecTask */
  ThermalTask_Run(argument);
  /* USER CODE END ThermalTecTask */
}

/* USER CODE BEGIN Header_StartSupervisionDDMTask */
/**
* @brief Function implementing the SupervisionDDMTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSupervisionDDMTask */
void StartSupervisionDDMTask(void *argument)
{
  /* USER CODE BEGIN SupervisionDDMTask */
  SupervisionTask_Run(argument);
  /* USER CODE END SupervisionDDMTask */
}

/* USER CODE BEGIN Header_StorageTask */
/**
* @brief Function implementing the StorageTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StorageTask */
void StorageTask (void *argument)
{
  /* USER CODE BEGIN StorageTask */
  StorageTask_Run(argument);
  /* USER CODE END StorageTask */
}

/* USER CODE BEGIN Header_StartTask08 */
/**
* @brief Function implementing the MsmTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask08 */
void StartTask08(void *argument)
{
  /* USER CODE BEGIN MsmTask */
  MsmTask_Run(argument);
  /* USER CODE END MsmTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

