/*
 *  @file Button.h
 *
 *  Created on: 10-Jan-2026
 *      Author: Priyanshu Roy
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "main.h"
#include "cmsis_os.h"

typedef enum {
  BTN_1 = 0,
  BTN_2,
  BTN_3,
  TOTAL_BTNS
}BTN_id_e;

typedef enum {
  // NO_BTN_EVENT = 0,
  SINGLE_PRESS = 0, // Press and release within 300 ms
  DOUBLE_PRESS, // Two presses within 500 ms
  TRIPLE_PRESS, // Three presses within 700 ms
  LONG_PRESS, // Hold longer than 1500 ms
  TOTAL_BTN_EVENTS
}BTN_event_e;

typedef struct {
  BTN_id_e id;
  BTN_event_e type;
  uint32_t timestamp;
}BTN_event_t;

typedef struct {
  BTN_id_e id;
  GPIO_TypeDef * port;
  uint16_t pin;
  
  // State Tracking
  GPIO_PinState last_state;           // Last read GPIO level (for edge detection)
  uint32_t last_time;       // Timestamp of the last state change (for debounce/hold)
  
  // Multi-press Tracking
  int click_count;          // How many clicks recorded so far
  uint32_t first_click_time;// When the very first click in a sequence started
  bool is_long_press_sent;  // Flag to prevent sending multiple Long Press events while holding

  GPIO_PinState last_raw_state; // The actual pin level last read
  uint32_t debounce_time;      // The last time the pin physically moved
  osMessageQueueId_t queue_handler;
}Button_t;

void Button_ctor(Button_t * const me, BTN_id_e id, GPIO_TypeDef * port, uint16_t pin,osMessageQueueId_t queue_handler);
void Button_read(Button_t * const me);

#endif /* BUTTON_H_ */
