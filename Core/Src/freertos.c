/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Button.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
static Button_t Buttons[TOTAL_BTNS];
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for BTN_IN_Thread */
osThreadId_t BTN_IN_ThreadHandle;
const osThreadAttr_t BTN_IN_Thread_attributes = { .name = "BTN_IN_Thread",
		.stack_size = 1024 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for MENU_Thread */
osThreadId_t MENU_ThreadHandle;
const osThreadAttr_t MENU_Thread_attributes = { .name = "MENU_Thread",
		.stack_size = 1024 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for DISP_MGR_Thread */
osThreadId_t DISP_MGR_ThreadHandle;
const osThreadAttr_t DISP_MGR_Thread_attributes = { .name = "DISP_MGR_Thread",
		.stack_size = 1024 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for button_event_queue */
osMessageQueueId_t button_event_queueHandle;
const osMessageQueueAttr_t button_event_queue_attributes = { .name =
		"button_event_queue" };
/* Definitions for display_pattern_queue */
osMessageQueueId_t display_pattern_queueHandle;
const osMessageQueueAttr_t display_pattern_queue_attributes = { .name =
		"display_pattern_queue" };
/* Definitions for shiftreg_mutex */
osMutexId_t shiftreg_mutexHandle;
const osMutexAttr_t shiftreg_mutex_attributes = { .name = "shiftreg_mutex" };

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void ButtonInputTask(void *argument);
void MenuLogicTask(void *argument);
void DisplayManagerTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */
	/* Create the mutex(es) */
	/* creation of shiftreg_mutex */
	shiftreg_mutexHandle = osMutexNew(&shiftreg_mutex_attributes);

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the queue(s) */
	/* creation of button_event_queue */
	button_event_queueHandle = osMessageQueueNew(16, sizeof(BTN_event_t),
			&button_event_queue_attributes);

	/* creation of display_pattern_queue */
	display_pattern_queueHandle = osMessageQueueNew(16, sizeof(uint16_t),
			&display_pattern_queue_attributes);

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of BTN_IN_Thread */
	BTN_IN_ThreadHandle = osThreadNew(ButtonInputTask, NULL,
			&BTN_IN_Thread_attributes);

	/* creation of MENU_Thread */
	MENU_ThreadHandle = osThreadNew(MenuLogicTask, NULL,
			&MENU_Thread_attributes);

	/* creation of DISP_MGR_Thread */
	DISP_MGR_ThreadHandle = osThreadNew(DisplayManagerTask, NULL,
			&DISP_MGR_Thread_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_ButtonInputTask */
/**
 * @brief  Function implementing the BTN_IN_Thread thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_ButtonInputTask */
void ButtonInputTask(void *argument) {
	/* USER CODE BEGIN ButtonInputTask */
	Button_ctor(&Buttons[0], BTN_1, BTN_1_GPIO_Port, BTN_1_Pin,
			button_event_queueHandle);
	/* Infinite loop */
	for (;;) {
		Button_read(&Buttons[0]);
		osDelay(10);
	}
	/* USER CODE END ButtonInputTask */
}

/* USER CODE BEGIN Header_MenuLogicTask */
/**
 * @brief Function implementing the MENU_Thread thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_MenuLogicTask */
void MenuLogicTask(void *argument) {
	/* USER CODE BEGIN MenuLogicTask */
	osStatus_t status;
	BTN_event_t event;
	/* Infinite loop */
	for (;;) {
		status = osMessageQueueGet(button_event_queueHandle, (void*) &event, 0,
		osWaitForever);
		if (status == osOK) {
			HAL_GPIO_TogglePin(U_LED_GPIO_Port, U_LED_Pin);
			log_message("MenuLogic", LOG_INFO, "BTN:%d EVT:%d TS:%lu", event.id,
					event.type, event.timestamp);
		}
	}
	/* USER CODE END MenuLogicTask */
}

/* USER CODE BEGIN Header_DisplayManagerTask */
/**
 * @brief Function implementing the DISP_MGR_Thread thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_DisplayManagerTask */
void DisplayManagerTask(void *argument) {
	/* USER CODE BEGIN DisplayManagerTask */
	/* Infinite loop */
	for (;;) {
		HAL_GPIO_TogglePin(U_LED_GPIO_Port, U_LED_Pin);
		osDelay(1000);
	}
	/* USER CODE END DisplayManagerTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

