/*
 * Menu.c
 *
 *  Created on: 11-Jan-2026
 *      Author: Priyanshu Roy
 */

#include "Menu.h"
#include "debug_logger.h"

#define Firmware_V_MAJOR	10
#define Firmware_V_Minor	5
#define FIRMWARE_V_Disp		(uint16_t)(Firmware_V_MAJOR<<8 | (Firmware_V_Minor&0x1f))

#define DEFAULT_BRIGHTNESS	5
#define DEFAULT_PATTERN		0
#define MAX_BRIGHTNESS		10
#define MIN_BRIGHTNESS		0

static char *const tag = "Menu";

// Predefined LED patterns for Manual Mode cycling
static uint16_t LED_PATTERNS[] = {
	0x0001, // Pattern 1
	0x0003, // Pattern 2
	0x0007, // Pattern 3
	0x000F, // Pattern 4
	0x001F, // Pattern 5
	0x003F, // Pattern 6
	0x007F, // Pattern 7
	0x00FF, // Pattern 8
	0x01FF, // Pattern 9
	0x03FF, // Pattern 10
	0x07FF, // Pattern 11
	0x0FFF, // Pattern 12
	0x1FFF, // Pattern 13
	0x3FFF, // Pattern 14
	0x7FFF, // Pattern 15
	0xFFFF  // Pattern 16
};
#define TOTAL_PATTERNS (sizeof(LED_PATTERNS)/sizeof(LED_PATTERNS[0]))

static uint16_t MENU_TO_PAGES[TOTAL_PAGES] =
{
	0x1000, 	// BRIGHTNESS_PAGE = 0
	0x2000, 	// MODE_SELECT_PAGE,
	0x4000,		// INFO_PAGE,
	0x8000,		// RESET_PAGE,
	0x200f,		// MODE_MANUAL_PAGE,
	0x20f0,		// MODE_AUTO_PAGE,
	0x03ff,		// BRIGHTNESS_SETTING,// mask
	0xffff,		// MANUAL_MODE,
	0xffff,		// AUTO_MODE,
	FIRMWARE_V_Disp,// FIRMWARE_VER,
	0x80FF		// RESET_CONFIRM
};

// Menu state variables
typedef struct {
	uint8_t brightness;
	uint8_t current_pattern_index;
	bool is_auto_mode;
	bool is_powered_on;
	Menu_State_e saved_mode_selection; // To track Manual vs Auto in Mode Select
} Menu_Settings_t;

static Menu_Settings_t menu_settings;

// Forward declarations for helper functions
static void send_display_update(Menu_t * const me, uint16_t pattern, uint8_t brightness);
static uint16_t get_brightness_pattern(uint8_t brightness);

void Menu_ctor(Menu_t * const me, osMessageQueueId_t queue_handler) {
	me->current_page = BRIGHTNESS_PAGE;
	me->pattern = MENU_TO_PAGES[BRIGHTNESS_PAGE];
	me->queue_handler = queue_handler;

	// Initialize default settings
	menu_settings.brightness = DEFAULT_BRIGHTNESS;
	menu_settings.current_pattern_index = DEFAULT_PATTERN;
	menu_settings.is_auto_mode = false;
	menu_settings.is_powered_on = true;
	menu_settings.saved_mode_selection = MODE_MANUAL_PAGE;

	// Send initial display state
	send_display_update(me, me->pattern, menu_settings.brightness);

	log_message(tag, LOG_INFO, "Menu initialized - Page: %d, Brightness: %d",
	            me->current_page, menu_settings.brightness);
}

static void send_display_update(Menu_t * const me, uint16_t pattern, uint8_t brightness) {
	Display_update_data_t display_data;
	display_data.data = pattern;
	display_data.brightness = brightness;

	osMessageQueuePut(me->queue_handler, &display_data, 0U, 0U);
}

static uint16_t get_brightness_pattern(uint8_t brightness) {
	// Create pattern based on brightness level (0-10)
	// 0 = 0x0000, 1 = 0x0001, 2 = 0x0003, ..., 10 = 0x07FF
	if (brightness == 0) return 0x0000;
	return ((1 << brightness) - 1) & MENU_TO_PAGES[BRIGHTNESS_SETTING]; // mask the output
}

static void handle_main_menu(Menu_t * const me, const BTN_event_t event) {
	switch(event.type) {
		case SINGLE_PRESS:
			if (event.id == BTN_1) {
				// Move selection forward
				if (me->current_page < RESET_PAGE) {
					me->current_page++;
				} else {
					me->current_page = BRIGHTNESS_PAGE;
				}
				me->pattern = MENU_TO_PAGES[me->current_page];
				send_display_update(me, me->pattern, menu_settings.brightness);
				log_message(tag, LOG_INFO, "Main Menu: Selected page %d", me->current_page);
			}
			else if (event.id == BTN_2) {
				// Enter selected item
				Menu_State_e next_page = me->current_page;

				switch(me->current_page) {
					case BRIGHTNESS_PAGE:
						next_page = BRIGHTNESS_SETTING;
						break;
					case MODE_SELECT_PAGE:
						next_page = menu_settings.saved_mode_selection;
						break;
					case INFO_PAGE:
						next_page = FIRMWARE_VER;
						break;
					case RESET_PAGE:
						next_page = RESET_CONFIRM;
					default:
						break;
				}

				me->current_page = next_page;
				me->pattern = MENU_TO_PAGES[me->current_page];
				send_display_update(me, me->pattern, menu_settings.brightness);
				log_message(tag, LOG_INFO, "Main Menu: Entered page %d", me->current_page);
			}
			else if (event.id == BTN_3) {
				// Move selection backward
				if (me->current_page > BRIGHTNESS_PAGE) {
					me->current_page--;
				} else {
					me->current_page = RESET_PAGE;
				}
				me->pattern = MENU_TO_PAGES[me->current_page];
				send_display_update(me, me->pattern, menu_settings.brightness);
				log_message(tag, LOG_INFO, "Main Menu: Selected page %d", me->current_page);
			}
			break;

		case LONG_PRESS:
			if (event.id == BTN_3) {
				// Power off
				menu_settings.is_powered_on = false;
				send_display_update(me, 0x0000, 0);
				log_message(tag, LOG_INFO, "Power OFF");
			}
			break;

		default:
			break;
	}
}

static void handle_brightness_setting(Menu_t * const me, const BTN_event_t event) {
	if (event.type == SINGLE_PRESS) {
		if (event.id == BTN_2) {
			// Increase brightness
			if (menu_settings.brightness < MAX_BRIGHTNESS) {
				menu_settings.brightness++;
				me->pattern = get_brightness_pattern(menu_settings.brightness);
				send_display_update(me, me->pattern, menu_settings.brightness);
				log_message(tag, LOG_INFO, "Brightness increased to %d", menu_settings.brightness);
			}
		}
		else if (event.id == BTN_3) {
			// Decrease brightness
			if (menu_settings.brightness > MIN_BRIGHTNESS) {
				menu_settings.brightness--;
				me->pattern = get_brightness_pattern(menu_settings.brightness);
				send_display_update(me, me->pattern, menu_settings.brightness);
				log_message(tag, LOG_INFO, "Brightness decreased to %d", menu_settings.brightness);
			}
		}
		else if (event.id == BTN_1) {
			// Return to main menu
			me->current_page = BRIGHTNESS_PAGE;
			me->pattern = MENU_TO_PAGES[me->current_page];
			send_display_update(me, me->pattern, menu_settings.brightness);
			log_message(tag, LOG_INFO, "Returned to Main Menu from Brightness");
		}
	}
}

static void handle_mode_select(Menu_t * const me, const BTN_event_t event) {
	if (event.type == SINGLE_PRESS) {
		if (event.id == BTN_1) {
			// Toggle Manual or Auto
			if (menu_settings.saved_mode_selection == MODE_MANUAL_PAGE) {
				menu_settings.saved_mode_selection = MODE_AUTO_PAGE;
				me->pattern = MENU_TO_PAGES[MODE_AUTO_PAGE];
			} else {
				menu_settings.saved_mode_selection = MODE_MANUAL_PAGE;
				me->pattern = MENU_TO_PAGES[MODE_MANUAL_PAGE];
			}
			send_display_update(me, me->pattern, menu_settings.brightness);
			log_message(tag, LOG_INFO, "Mode toggled to %s",
			            (menu_settings.saved_mode_selection == MODE_MANUAL_PAGE) ? "Manual" : "Auto");
		}
		else if (event.id == BTN_2) {
			// Enter selected mode
			if (menu_settings.saved_mode_selection == MODE_MANUAL_PAGE) {
				me->current_page = MANUAL_MODE;
				me->pattern = LED_PATTERNS[menu_settings.current_pattern_index];
			} else {
				me->current_page = AUTO_MODE;
				me->pattern = LED_PATTERNS[menu_settings.current_pattern_index];
				menu_settings.is_auto_mode = true;
			}
			send_display_update(me, me->pattern, menu_settings.brightness);
			log_message(tag, LOG_INFO, "Entered %s mode",
			            (me->current_page == MANUAL_MODE) ? "Manual" : "Auto");
		}
		else if (event.id == BTN_3) {
			// Cancel and return to main menu
			me->current_page = MODE_SELECT_PAGE;
			me->pattern = MENU_TO_PAGES[MODE_SELECT_PAGE];
			send_display_update(me, me->pattern, menu_settings.brightness);
			log_message(tag, LOG_INFO, "Cancelled mode selection");
		}
	}
}

static void handle_manual_mode(Menu_t * const me, const BTN_event_t event) {
	if (event.type == SINGLE_PRESS) {
		if (event.id == BTN_1) {
			// Cycle LED patterns
			menu_settings.current_pattern_index++;
			if (menu_settings.current_pattern_index >= TOTAL_PATTERNS) {
				menu_settings.current_pattern_index = 0;
			}
			me->pattern = LED_PATTERNS[menu_settings.current_pattern_index];
			send_display_update(me, me->pattern, menu_settings.brightness);
			log_message(tag, LOG_INFO, "Manual Mode: Pattern %d selected",
			            menu_settings.current_pattern_index);
		}
		else if (event.id == BTN_2) {
			// Save and return to main menu
			me->current_page = MODE_SELECT_PAGE;
			me->pattern = MENU_TO_PAGES[MODE_SELECT_PAGE];
			send_display_update(me, me->pattern, menu_settings.brightness);
			log_message(tag, LOG_INFO, "Manual Mode: Saved pattern %d",
			            menu_settings.current_pattern_index);
		}
		else if (event.id == BTN_3) {
			// Cancel - return to mode select without saving
			me->current_page = MODE_SELECT_PAGE;
			me->pattern = MENU_TO_PAGES[MODE_SELECT_PAGE];
			send_display_update(me, me->pattern, menu_settings.brightness);
			log_message(tag, LOG_INFO, "Manual Mode: Cancelled");
		}
	}
}

static void handle_auto_mode(Menu_t * const me, const BTN_event_t event) {
	if (event.type == SINGLE_PRESS && event.id == BTN_1) {
		// Exit Auto and return to mode select
		menu_settings.is_auto_mode = false;
		me->current_page = MODE_MANUAL_PAGE;
		me->pattern = MENU_TO_PAGES[MODE_MANUAL_PAGE];
		send_display_update(me, me->pattern, menu_settings.brightness);
		log_message(tag, LOG_INFO, "Auto Mode: Exited");
	}
	// Note: Auto mode cycling will be handled by a separate timer task
}

static void handle_info(Menu_t * const me, const BTN_event_t event) {
	if (event.type == SINGLE_PRESS) {
		if (event.id == BTN_1) {
			// Next info screen (currently only firmware version)
			// Could add more info screens here in the future
			log_message(tag, LOG_INFO, "Info: Next screen");
		}
		else if (event.id == BTN_3) {
			// Return to main menu
			me->current_page = INFO_PAGE;
			me->pattern = MENU_TO_PAGES[INFO_PAGE];
			send_display_update(me, me->pattern, menu_settings.brightness);
			log_message(tag, LOG_INFO, "Info: Returned to Main Menu");
		}
	}
}

static void handle_reset(Menu_t * const me, const BTN_event_t event) {
	if (event.type == DOUBLE_PRESS && event.id == BTN_2) {
		// Confirm reset - restore defaults
		menu_settings.brightness = DEFAULT_BRIGHTNESS;
		menu_settings.current_pattern_index = DEFAULT_PATTERN;
		menu_settings.is_auto_mode = false;
		menu_settings.saved_mode_selection = MODE_MANUAL_PAGE;

		me->current_page = BRIGHTNESS_PAGE;
		me->pattern = MENU_TO_PAGES[BRIGHTNESS_PAGE];
		send_display_update(me, me->pattern, menu_settings.brightness);

		log_message(tag, LOG_INFO, "Reset: Settings restored to defaults");
	}
	else if (event.type == SINGLE_PRESS && event.id == BTN_3) {
		// Cancel reset
		me->current_page = RESET_PAGE;
		me->pattern = MENU_TO_PAGES[RESET_PAGE];
		send_display_update(me, me->pattern, menu_settings.brightness);
		log_message(tag, LOG_INFO, "Reset: Cancelled");
	}
}

static void handle_power_off(Menu_t * const me, const BTN_event_t event) {
	if (event.type == LONG_PRESS && event.id == BTN_1) {
		// Restart/power on
		menu_settings.is_powered_on = true;
		me->current_page = BRIGHTNESS_PAGE;
		me->pattern = MENU_TO_PAGES[BRIGHTNESS_PAGE];
		send_display_update(me, me->pattern, menu_settings.brightness);
		log_message(tag, LOG_INFO, "Power ON");
	}
}

void Menu_process_input(Menu_t * const me, const BTN_event_t event) {
	// Check if powered off - only respond to BTN1 Long press
	if (!menu_settings.is_powered_on) {
		handle_power_off(me, event);
		return;
	}

	// Route to appropriate handler based on current page
	switch(me->current_page) {
		case BRIGHTNESS_PAGE:
		case MODE_SELECT_PAGE:
		case INFO_PAGE:
		case RESET_PAGE:
			handle_main_menu(me, event);
			break;

		case BRIGHTNESS_SETTING:
			handle_brightness_setting(me, event);
			break;

		case MODE_MANUAL_PAGE:
		case MODE_AUTO_PAGE:
			handle_mode_select(me, event);
			break;

		case MANUAL_MODE:
			handle_manual_mode(me, event);
			break;

		case AUTO_MODE:
			handle_auto_mode(me, event);
			break;

		case FIRMWARE_VER:
			handle_info(me, event);
			break;

		case RESET_CONFIRM:
			handle_reset(me, event);
			break;

		default:
			log_message(tag, LOG_WARN, "Unknown page state: %d", me->current_page);
			break;
	}
}

// Additional helper function to get current auto mode state
bool Menu_is_auto_mode_active(void) {
	return menu_settings.is_auto_mode;
}

// Helper function to cycle pattern in auto mode
void Menu_auto_cycle_pattern(Menu_t * const me) {
	if (menu_settings.is_auto_mode && me->current_page == AUTO_MODE) {
		menu_settings.current_pattern_index++;
		if (menu_settings.current_pattern_index >= TOTAL_PATTERNS) {
			menu_settings.current_pattern_index = 0;
		}
		me->pattern = LED_PATTERNS[menu_settings.current_pattern_index];
		send_display_update(me, me->pattern, menu_settings.brightness);
	}
}
