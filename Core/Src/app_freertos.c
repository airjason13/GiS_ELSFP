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
  .stack_size = 128 * 4
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
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

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

  /* USER CODE BEGIN RTOS_THREADS */
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
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
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
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
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
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
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
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
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
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
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
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
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
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StorageTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

