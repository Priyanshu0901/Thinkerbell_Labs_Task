#include "Button.h"

#define DEBOUNCE_MS 30
#define SINGLE_WINDOW     (300)
#define DOUBLE_WINDOW     (500)
#define TRIPLE_WINDOW     (700)
#define LONG_PRESS_TIME   (1500)

//static char *const tag = "Button";

void Button_ctor(Button_t *const me, BTN_id_e id, GPIO_TypeDef *port,
		uint16_t pin, osMessageQueueId_t queue_handler) {
	me->id = id;
	me->port = port;
	me->pin = pin;
	me->queue_handler = queue_handler;

	GPIO_PinState init_state = HAL_GPIO_ReadPin(me->port, me->pin);
	me->last_state = init_state;
	me->last_raw_state = init_state;

	me->last_time = HAL_GetTick();
	me->debounce_time = HAL_GetTick();

	me->click_count = 0;
	me->is_long_press_sent = false;
}

void Button_read(Button_t *const me) {
	uint32_t now = HAL_GetTick();
	GPIO_PinState current_raw_state = HAL_GPIO_ReadPin(me->port, me->pin);

	// --- DEBOUNCE ---
	if (current_raw_state != me->last_raw_state) {
		me->debounce_time = now; // Reset debounce timer on any flicker
	}
	me->last_raw_state = current_raw_state;

	// Process the edge (after stability)
	if ((now - me->debounce_time) > DEBOUNCE_MS) {
		if (current_raw_state != me->last_state) {
			// EDGE DETECTED
			if (current_raw_state == GPIO_PIN_RESET) { // FALLING (Press -> Active Low)
				if (me->click_count == 0)
					me->first_click_time = now;
				me->click_count++;
				me->last_time = now; // Start "hold" timer
				me->is_long_press_sent = false;
			} else { // RISING (Release)
				if (me->is_long_press_sent) {
					me->click_count = 0; // It was a long press, reset count
				}
				me->last_time = now; // Start "idle" timer
			}
			me->last_state = current_raw_state;
		}
	}

	// --- STATE DECISION ---

	// Check for Long Press (While held down)
	if (me->last_state == GPIO_PIN_RESET && !me->is_long_press_sent) {
		if ((now - me->last_time) > LONG_PRESS_TIME) {
//			log_message(tag, LOG_INFO, "Long Press on BTN %d", me->id);
			BTN_event_t event = {.id = me->id,.type = LONG_PRESS,.timestamp = now};
			osMessageQueuePut(me->queue_handler, &event, 0U, 0U);
			me->is_long_press_sent = true;
		}
	}

	// Check for Multi-Click Decisions (While released)
	if (me->last_state == GPIO_PIN_SET && me->click_count > 0) {
		uint32_t idle_time = now - me->last_time;
		uint32_t total_time = now - me->first_click_time;
		BTN_event_t event = {.id = me->id,.timestamp = now};
		bool to_send = false;

		// Triple Press (Immediate trigger on 3rd release)
		if (me->click_count == 3) {
//			log_message(tag, LOG_INFO, "Triple Press on BTN %d", me->id);
			event.type = TRIPLE_PRESS;
			to_send = true;
			me->click_count = 0;
		}
		// Double Press (Trigger if 2 clicks exist and window for 3rd has closed)
		else if (me->click_count == 2
				&& (total_time > TRIPLE_WINDOW || idle_time > (DOUBLE_WINDOW - SINGLE_WINDOW))) {
//			log_message(tag, LOG_INFO, "Double Press on BTN %d", me->id);
			event.type = DOUBLE_PRESS;
			to_send = true;
			me->click_count = 0;
		}
		// Single Press (Trigger if 1 click exists and window for 2nd has closed)
		else if (me->click_count == 1
				&& (total_time > DOUBLE_WINDOW || idle_time > SINGLE_WINDOW)) {
//			log_message(tag, LOG_INFO, "Single Press on BTN %d", me->id);
			event.type = SINGLE_PRESS;
			to_send = true;
			me->click_count = 0;
		}

		if(to_send) osMessageQueuePut(me->queue_handler, &event, 0U, 20U);
	}
}
