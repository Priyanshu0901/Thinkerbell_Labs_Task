/*
 * Display.h
 *
 *  Created on: 11-Jan-2026
 *      Author: Priyanshu Roy
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#include "main.h"
#include "SN74HC595.h"
#include "cmsis_os.h"
#include "Menu.h"

// Display manager state structure
typedef struct {
	SN74HC595_t *shift_register;           // Pointer to shift register driver
	osMessageQueueId_t update_queue;      // Queue for receiving display updates
	osMutexId_t hardware_mutex;            // Mutex for hardware access

	// Current display state
	uint16_t current_pattern;              // Currently displayed pattern
	uint8_t current_brightness;            // Current brightness level
	bool is_enabled;                       // Display on/off state
} Display_Manager_t;


void Display_ctor(Display_Manager_t *const me, SN74HC595_t *shift_reg,
		osMessageQueueId_t queue, osMutexId_t mutex);


bool Display_update(Display_Manager_t *const me,
		const Display_update_data_t *update);


bool Display_set_pattern(Display_Manager_t *const me, uint16_t pattern);
bool Display_set_brightness(Display_Manager_t *const me, uint8_t brightness);

void Display_disable(Display_Manager_t *const me);
void Display_enable(Display_Manager_t *const me);


void Display_clear(Display_Manager_t *const me);
void Display_get_state(Display_Manager_t *const me, uint16_t *pattern,
		uint8_t *brightness);


bool Display_is_enabled(Display_Manager_t *const me);

#endif /* INC_DISPLAY_H_ */
