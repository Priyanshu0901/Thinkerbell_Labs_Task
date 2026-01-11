#include "Button.h"

#define DEBOUNCE_MS 10
#define SINGLE_WINDOW     (300)
#define DOUBLE_WINDOW     (500)
#define TRIPLE_WINDOW     (700)
#define LONG_PRESS_TIME   (1500)

static char * const tag = "Button";

void Button_ctor(Button_t *const me, BTN_id_e id, GPIO_TypeDef *port, uint16_t pin) {
	me->id = id;
	me->port = port;
	me->pin = pin;

	me->last_state = HAL_GPIO_ReadPin(me->port, me->pin);
	me->last_time = HAL_GetTick();
	me->click_count = 0;
	me->is_long_press_sent = false;
	me->waiting_for_release = false;
}

void update_button_state(Button_t *const me, int state, int timestamp) {
	// 1. Detect Edges
	bool falling_edge = (me->last_state == GPIO_PIN_SET && state == GPIO_PIN_RESET); // Pressed
	bool rising_edge = (me->last_state == GPIO_PIN_RESET && state == GPIO_PIN_SET); // Released

	if (falling_edge) {
		if (me->click_count == 0) {
			me->first_click_time = timestamp;
		}
		me->click_count++;
		me->last_time = timestamp;
		me->is_long_press_sent = false;
	}

	if (rising_edge) {
		// If it was a long press, we don't want to count the release as a click
		if (me->is_long_press_sent) {
			me->click_count = 0;
		}
		me->last_time = timestamp;
	}

	// 2. Continuous Long Press Check (While button is held down)
	if (state == GPIO_PIN_RESET && !me->is_long_press_sent) {
		if ((timestamp - me->last_time) > LONG_PRESS_TIME) {
			// send_event_to_queue(me->id, LONG_PRESS, timestamp);
			log_message(tag, LOG_INFO, "Long Press detected on %d",me->id);
			me->is_long_press_sent = true;
			// Note: click_count isn't cleared until release
		}
	}

	// 3. The "Decision" Logic (While button is released)
	if (state == GPIO_PIN_SET && me->click_count > 0) {
		uint32_t time_since_start = timestamp - me->first_click_time;

		// Triple Press: Trigger immediately on 3rd click release if within window
		if (me->click_count == 3) {
			if (time_since_start <= TRIPLE_WINDOW) {
				log_message(tag, LOG_INFO, "Triple Press detected on %d",me->id);
				// send_event_to_queue(me->id, TRIPLE_PRESS, timestamp);
			}
			me->click_count = 0;
		}
		// Double Press: Wait for Triple window to expire
		else if (me->click_count == 2 && time_since_start > TRIPLE_WINDOW) {
			log_message(tag, LOG_INFO, "Double Press detected on %d",me->id);
			//  send_event_to_queue(me->id, DOUBLE_PRESS, timestamp);
			me->click_count = 0;
		}
		// Single Press: Wait for Double window to expire
		else if (me->click_count == 1 && time_since_start > DOUBLE_WINDOW) {
			log_message(tag, LOG_INFO, "Single Press detected on %d",me->id);
			//  send_event_to_queue(me->id, SINGLE_PRESS, timestamp);
			me->click_count = 0;
		}
	}

	me->last_state = state;
}

void Button_read(Button_t *const me) {
	int current_time = HAL_GetTick();

	if ((current_time - me->last_time) < DEBOUNCE_MS)
		return;

	int current_state = HAL_GPIO_ReadPin(me->port, me->pin);
	update_button_state(me, current_state, current_time);
}
