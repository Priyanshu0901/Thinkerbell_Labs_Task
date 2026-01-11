/*
 * Display.c
 *
 *  Created on: 12-Jan-2026
 *      Author: rayv_mini_pc
 */

/*
 * Display.c
 *
 *  Created on: 11-Jan-2026
 *      Author: Priyanshu Roy
 *
 *  Description: Display manager implementation
 */

#include "Display.h"
#include "debug_logger.h"

#define MAX_BRIGHTNESS 10
#define MIN_BRIGHTNESS 0
#define MUTEX_TIMEOUT_MS 100

static char *const tag = "Display";

void Display_ctor(Display_Manager_t * const me,
                  SN74HC595_t *shift_reg,
                  osMessageQueueId_t queue,
                  osMutexId_t mutex) {

	// Store references
	me->shift_register = shift_reg;
	me->update_queue = queue;
	me->hardware_mutex = mutex;

	// Initialize state
	me->current_pattern = 0x0000;
	me->current_brightness = 5;  // Default medium brightness
	me->is_enabled = true;

	log_message(tag, LOG_INFO, "Display Manager initialized");
}

bool Display_update(Display_Manager_t * const me, const Display_update_data_t *update) {
	if (update == NULL) {
		log_message(tag, LOG_ERROR, "Update data is NULL");
		return false;
	}

	// Validate brightness
	if (update->brightness > MAX_BRIGHTNESS) {
		log_message(tag, LOG_WARN, "Brightness %d exceeds max %d, clamping",
		            update->brightness, MAX_BRIGHTNESS);
	}

	// Acquire hardware mutex
	osStatus_t status = osMutexAcquire(me->hardware_mutex, MUTEX_TIMEOUT_MS);
	if (status != osOK) {
		log_message(tag, LOG_ERROR, "Failed to acquire mutex (status: %d)", status);
		return false;
	}

	// Update shift register
	SN74HC595_update(me->shift_register, update->data, update->brightness);

	// Update local state
	me->current_pattern = update->data;
	me->current_brightness = update->brightness;

	// Release mutex
	osMutexRelease(me->hardware_mutex);

	log_message(tag, LOG_DEBUG, "Display updated: Pattern=0x%04X, Brightness=%d",
	            update->data, update->brightness);

	return true;
}

bool Display_set_pattern(Display_Manager_t * const me, uint16_t pattern) {
	// Acquire hardware mutex
	osStatus_t status = osMutexAcquire(me->hardware_mutex, MUTEX_TIMEOUT_MS);
	if (status != osOK) {
		log_message(tag, LOG_ERROR, "Failed to acquire mutex");
		return false;
	}

	// Update pattern only
	SN74HC595_write(me->shift_register, pattern);
	me->current_pattern = pattern;

	// Release mutex
	osMutexRelease(me->hardware_mutex);

	log_message(tag, LOG_DEBUG, "Pattern set to 0x%04X", pattern);

	return true;
}

bool Display_set_brightness(Display_Manager_t * const me, uint8_t brightness) {
	// Validate brightness
	if (brightness > MAX_BRIGHTNESS) {
		brightness = MAX_BRIGHTNESS;
		log_message(tag, LOG_WARN, "Brightness clamped to %d", MAX_BRIGHTNESS);
	}

	// Acquire hardware mutex
	osStatus_t status = osMutexAcquire(me->hardware_mutex, MUTEX_TIMEOUT_MS);
	if (status != osOK) {
		log_message(tag, LOG_ERROR, "Failed to acquire mutex");
		return false;
	}

	// Update brightness only
	SN74HC595_set_brightness(me->shift_register, brightness);
	me->current_brightness = brightness;

	// Release mutex
	osMutexRelease(me->hardware_mutex);

	log_message(tag, LOG_DEBUG, "Brightness set to %d", brightness);

	return true;
}

void Display_disable(Display_Manager_t * const me) {
	// Acquire hardware mutex
	osStatus_t status = osMutexAcquire(me->hardware_mutex, MUTEX_TIMEOUT_MS);
	if (status != osOK) {
		log_message(tag, LOG_ERROR, "Failed to acquire mutex for disable");
		return;
	}

	// Disable output
	SN74HC595_disable_output(me->shift_register);
	me->is_enabled = false;

	// Release mutex
	osMutexRelease(me->hardware_mutex);

	log_message(tag, LOG_INFO, "Display disabled");
}

void Display_enable(Display_Manager_t * const me) {
	// Acquire hardware mutex
	osStatus_t status = osMutexAcquire(me->hardware_mutex, MUTEX_TIMEOUT_MS);
	if (status != osOK) {
		log_message(tag, LOG_ERROR, "Failed to acquire mutex for enable");
		return;
	}

	// Enable output (restores previous brightness)
	SN74HC595_enable_output(me->shift_register);
	me->is_enabled = true;

	// Release mutex
	osMutexRelease(me->hardware_mutex);

	log_message(tag, LOG_INFO, "Display enabled - Brightness: %d", me->current_brightness);
}

void Display_clear(Display_Manager_t * const me) {
	// Acquire hardware mutex
	osStatus_t status = osMutexAcquire(me->hardware_mutex, MUTEX_TIMEOUT_MS);
	if (status != osOK) {
		log_message(tag, LOG_ERROR, "Failed to acquire mutex for clear");
		return;
	}

	// Clear shift register
	SN74HC595_clear(me->shift_register);
	me->current_pattern = 0x0000;

	// Release mutex
	osMutexRelease(me->hardware_mutex);

	log_message(tag, LOG_INFO, "Display cleared");
}

void Display_get_state(Display_Manager_t * const me, uint16_t *pattern, uint8_t *brightness) {
	if (pattern != NULL) {
		*pattern = me->current_pattern;
	}

	if (brightness != NULL) {
		*brightness = me->current_brightness;
	}
}

bool Display_is_enabled(Display_Manager_t * const me) {
	return me->is_enabled;
}
