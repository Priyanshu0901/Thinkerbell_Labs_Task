/*
 * Menu.h
 *
 *  Created on: 11-Jan-2026
 *      Author: Priyanshu Roy
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_

#include "main.h"
#include "Button.h"
#include "cmsis_os.h"

typedef enum{
	BRIGHTNESS_PAGE = 0,
	MODE_SELECT_PAGE,
	INFO_PAGE,
	RESET_PAGE,
	MODE_MANUAL_PAGE,
	MODE_AUTO_PAGE,
	BRIGHTNESS_SETTING,
	MANUAL_MODE,
	AUTO_MODE,
	FIRMWARE_VER,
	RESET_CONFIRM,
	TOTAL_PAGES
}Menu_State_e;

typedef struct{
	uint16_t data;
	uint8_t brightness;
}Display_update_data_t;

typedef struct {
	uint16_t pattern;
	Menu_State_e current_page;
	osMessageQueueId_t queue_handler;
}Menu_t;

void Menu_ctor(Menu_t * const me, osMessageQueueId_t queue_handler);
void Menu_process_input(Menu_t * const me, const BTN_event_t event);

// Additional helper functions
bool Menu_is_auto_mode_active(void);
void Menu_auto_cycle_pattern(Menu_t * const me);

#endif /* INC_MENU_H_ */
