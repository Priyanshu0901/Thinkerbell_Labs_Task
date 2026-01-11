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
	osMessageQueueId_t update_queue;       // Queue for receiving display updates
	osMutexId_t hardware_mutex;            // Mutex for hardware access

	// Current display state
	uint16_t current_pattern;              // Currently displayed pattern
	uint8_t current_brightness;            // Current brightness level
	bool is_enabled;                       // Display on/off state
} Display_Manager_t;

/**
 * @brief Initialize the display manager
 *
 * @param me Pointer to Display_Manager structure
 * @param shift_reg Pointer to initialized SN74HC595 driver
 * @param queue Queue handle for receiving display updates
 * @param mutex Mutex handle for hardware protection
 */
void Display_ctor(Display_Manager_t * const me,
                  SN74HC595_t *shift_reg,
                  osMessageQueueId_t queue,
                  osMutexId_t mutex);

/**
 * @brief Process a display update request
 *
 * @param me Pointer to Display_Manager structure
 * @param update Display update data (pattern + brightness)
 * @return true if update successful, false otherwise
 */
bool Display_update(Display_Manager_t * const me, const Display_update_data_t *update);

/**
 * @brief Set LED pattern without changing brightness
 *
 * @param me Pointer to Display_Manager structure
 * @param pattern 16-bit LED pattern
 * @return true if successful
 */
bool Display_set_pattern(Display_Manager_t * const me, uint16_t pattern);

/**
 * @brief Set brightness without changing pattern
 *
 * @param me Pointer to Display_Manager structure
 * @param brightness Brightness level 0-10
 * @return true if successful
 */
bool Display_set_brightness(Display_Manager_t * const me, uint8_t brightness);

/**
 * @brief Turn off display (power save mode)
 *
 * @param me Pointer to Display_Manager structure
 */
void Display_disable(Display_Manager_t * const me);

/**
 * @brief Turn on display (restore previous state)
 *
 * @param me Pointer to Display_Manager structure
 */
void Display_enable(Display_Manager_t * const me);

/**
 * @brief Clear all LEDs
 *
 * @param me Pointer to Display_Manager structure
 */
void Display_clear(Display_Manager_t * const me);

/**
 * @brief Get current display state
 *
 * @param me Pointer to Display_Manager structure
 * @param pattern Output: current pattern
 * @param brightness Output: current brightness
 */
void Display_get_state(Display_Manager_t * const me, uint16_t *pattern, uint8_t *brightness);

/**
 * @brief Check if display is enabled
 *
 * @param me Pointer to Display_Manager structure
 * @return true if enabled, false if disabled
 */
bool Display_is_enabled(Display_Manager_t * const me);
#endif /* INC_DISPLAY_H_ */
