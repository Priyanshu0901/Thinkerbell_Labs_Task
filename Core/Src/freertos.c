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
#include "Display.h"
#include "Button.h"
#include "Menu.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define AUTO_CYCLE_PERIOD_MS	2000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
static Button_t Buttons[TOTAL_BTNS];
static Menu_t Menu;
static SN74HC595_t ShiftRegister;
static Display_Manager_t DisplayManager;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for BTN_IN_Thread */
osThreadId_t BTN_IN_ThreadHandle;
const osThreadAttr_t BTN_IN_Thread_attributes = {
  .name = "BTN_IN_Thread",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MENU_Thread */
osThreadId_t MENU_ThreadHandle;
const osThreadAttr_t MENU_Thread_attributes = {
  .name = "MENU_Thread",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for DISP_MGR_Thread */
osThreadId_t DISP_MGR_ThreadHandle;
const osThreadAttr_t DISP_MGR_Thread_attributes = {
  .name = "DISP_MGR_Thread",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for button_event_queue */
osMessageQueueId_t button_event_queueHandle;
const osMessageQueueAttr_t button_event_queue_attributes = {
  .name = "button_event_queue"
};
/* Definitions for display_pattern_queue */
osMessageQueueId_t display_pattern_queueHandle;
const osMessageQueueAttr_t display_pattern_queue_attributes = {
  .name = "display_pattern_queue"
};
/* Definitions for shiftreg_mutex */
osMutexId_t shiftreg_mutexHandle;
const osMutexAttr_t shiftreg_mutex_attributes = {
  .name = "shiftreg_mutex"
};

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
  button_event_queueHandle = osMessageQueueNew (16, sizeof(BTN_event_t), &button_event_queue_attributes);

  /* creation of display_pattern_queue */
  display_pattern_queueHandle = osMessageQueueNew (16, sizeof(Display_update_data_t), &display_pattern_queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of BTN_IN_Thread */
  BTN_IN_ThreadHandle = osThreadNew(ButtonInputTask, NULL, &BTN_IN_Thread_attributes);

  /* creation of MENU_Thread */
  MENU_ThreadHandle = osThreadNew(MenuLogicTask, NULL, &MENU_Thread_attributes);

  /* creation of DISP_MGR_Thread */
  DISP_MGR_ThreadHandle = osThreadNew(DisplayManagerTask, NULL, &DISP_MGR_Thread_attributes);

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
void ButtonInputTask(void *argument)
{
  /* USER CODE BEGIN ButtonInputTask */
	Button_ctor(&Buttons[0], BTN_1, BTN_1_GPIO_Port, BTN_1_Pin,
			button_event_queueHandle);
	Button_ctor(&Buttons[1], BTN_2, BTN_2_GPIO_Port, BTN_2_Pin,
				button_event_queueHandle);
	Button_ctor(&Buttons[2], BTN_3, BTN_3_GPIO_Port, BTN_3_Pin,
				button_event_queueHandle);
	/* Infinite loop */
	for (;;) {
		for(int i = 0;i<TOTAL_BTNS;i++) { Button_read(&Buttons[i]); }
		osDelay(5);
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
void MenuLogicTask(void *argument)
{
  /* USER CODE BEGIN MenuLogicTask */
	osStatus_t status;
	BTN_event_t event;
	uint32_t last_auto_cycle_time = 0;

	// Initialize Menu
	Menu_ctor(&Menu, display_pattern_queueHandle);
	log_message("MenuLogic", LOG_INFO, "Menu Logic Task started");

	/* Infinite loop */
	for (;;) {
		// Check for button events (with timeout to allow auto-cycle checking)
		status = osMessageQueueGet(button_event_queueHandle, (void*) &event, 0, 100);

		if (status == osOK) {
			// Process button event
			log_message("MenuLogic", LOG_INFO, "BTN:%d EVT:%d TS:%lu",
			            event.id, event.type, event.timestamp);
			Menu_process_input(&Menu, event);
		}

		// Handle auto-mode pattern cycling (every 2 seconds)
		if (Menu_is_auto_mode_active()) {
			uint32_t current_time = HAL_GetTick();
			if ((current_time - last_auto_cycle_time) >= AUTO_CYCLE_PERIOD_MS) {
				Menu_auto_cycle_pattern(&Menu);
				last_auto_cycle_time = current_time;
				log_message("MenuLogic", LOG_DEBUG, "Auto mode: Pattern cycled");
			}
		} else {
			// Reset the timer when not in auto mode
			last_auto_cycle_time = HAL_GetTick();
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
void DisplayManagerTask(void *argument)
{
  /* USER CODE BEGIN DisplayManagerTask */
	osStatus_t status;
		Display_update_data_t display_data;

		// Initialize Shift Register
		// Pin mappings from main.h:
		// - SR_DATA: PA6
		// - SR_CLK: PA7
		// - RCLK: PB0
		// - SR_CLR: PB1
		// - OE: TIM2_CH1 (PWM)
		SN74HC595_ctor(&ShiftRegister,
		               SR_DATA_GPIO_Port, SR_DATA_Pin,      // Serial Data
		               SR_CLK_GPIO_Port, SR_CLK_Pin,        // Serial Clock
		               RCLK_GPIO_Port, RCLK_Pin,            // Register Clock (Latch)
		               SR_CLR_GPIO_Port, SR_CLR_Pin,        // Clear
		               &htim2, TIM_CHANNEL_1);              // PWM for OE (brightness)

		// Initialize Display Manager
		Display_ctor(&DisplayManager,
		             &ShiftRegister,
		             display_pattern_queueHandle,
		             shiftreg_mutexHandle);

		log_message("DisplayMgr", LOG_INFO, "Display Manager Task started");

		/* Infinite loop */
		for (;;) {
			// Wait for display update requests from Menu
			status = osMessageQueueGet(display_pattern_queueHandle,
			                           (void*) &display_data,
			                           0,
			                           osWaitForever);

			if (status == osOK) {
				// Process the display update
				if (Display_update(&DisplayManager, &display_data)) {
				} else {
					log_message("DisplayMgr", LOG_ERROR, "Display update failed");
				}
			}
		}
  /* USER CODE END DisplayManagerTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

