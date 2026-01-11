/*
 * SN74HC595.c
 *
 *  Created on: Jan-2026
 *      Author: Priyanshu Roy
 *
 *  Description: Driver implementation for SN74HC595 shift register
 */

#include "SN74HC595.h"
#include "debug_logger.h"

#define MAX_BRIGHTNESS 10
#define MIN_BRIGHTNESS 0

// PWM timer period (assuming timer configured for 1kHz PWM)
// Adjust ARR value based on your TIM2 configuration
#define PWM_PERIOD 100  // 0-100 for percentage-based duty cycle

static char *const tag = "SR595";

// Helper function to pulse clock
static inline void pulse_clock(GPIO_TypeDef *port, uint16_t pin) {
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
	// Small delay for setup time (optional, depends on clock speed)
	// For 96MHz STM32F411, GPIO is fast enough without explicit delay
	__NOP();
	__NOP();
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

// Helper function to latch data to output
static inline void pulse_latch(GPIO_TypeDef *port, uint16_t pin) {
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
	__NOP();
	__NOP();
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

void SN74HC595_ctor(SN74HC595_t * const me,
                    GPIO_TypeDef *data_port, uint16_t data_pin,
                    GPIO_TypeDef *clk_port, uint16_t clk_pin,
                    GPIO_TypeDef *rclk_port, uint16_t rclk_pin,
                    GPIO_TypeDef *clr_port, uint16_t clr_pin,
                    TIM_HandleTypeDef *htim, uint32_t tim_channel) {

	// Store pin configurations
	me->ser_data_port = data_port;
	me->ser_data_pin = data_pin;
	me->ser_clk_port = clk_port;
	me->ser_clk_pin = clk_pin;
	me->rclk_port = rclk_port;
	me->rclk_pin = rclk_pin;
	me->ser_clr_port = clr_port;
	me->ser_clr_pin = clr_pin;
	me->htim = htim;
	me->tim_channel = tim_channel;

	// Initialize state
	me->current_data = 0x0000;
	me->current_brightness = 5; // Default medium brightness

	// Initialize GPIO states
	HAL_GPIO_WritePin(me->ser_data_port, me->ser_data_pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(me->ser_clk_port, me->ser_clk_pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(me->rclk_port, me->rclk_pin, GPIO_PIN_RESET);

	// SRCLR is active low - keep HIGH for normal operation
	HAL_GPIO_WritePin(me->ser_clr_port, me->ser_clr_pin, GPIO_PIN_SET);

	// Start PWM for brightness control (OE pin)
	// OE is active LOW, so higher duty cycle = dimmer LEDs
	HAL_TIM_PWM_Start(me->htim, me->tim_channel);
	SN74HC595_set_brightness(me, me->current_brightness);

	// Clear the shift register
	SN74HC595_clear(me);

	log_message(tag, LOG_INFO, "SN74HC595 initialized - Brightness: %d", me->current_brightness);
}

void SN74HC595_write(SN74HC595_t * const me, uint16_t data) {
	// Store current data
	me->current_data = data;

	// Shift out 16 bits, MSB first
	// Since we have two 8-bit shift registers cascaded:
	// First shift register gets bits 15-8
	// Second shift register gets bits 7-0

	for (int i = 15; i >= 0; i--) {
		// Set data bit
		GPIO_PinState bit_state = (data & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET;
		HAL_GPIO_WritePin(me->ser_data_port, me->ser_data_pin, bit_state);

		// Pulse serial clock to shift in the bit
		pulse_clock(me->ser_clk_port, me->ser_clk_pin);
	}

	// Pulse RCLK to latch the data to output registers
	pulse_latch(me->rclk_port, me->rclk_pin);

	log_message(tag, LOG_DEBUG, "Wrote data: 0x%04X", data);
}

void SN74HC595_set_brightness(SN74HC595_t * const me, uint8_t brightness) {
	// Clamp brightness to valid range
	if (brightness > MAX_BRIGHTNESS) {
		brightness = MAX_BRIGHTNESS;
	}

	me->current_brightness = brightness;

	// Calculate PWM duty cycle
	// OE is active LOW:
	// - 0% duty cycle (CCR=0) = Maximum brightness (OE always LOW)
	// - 100% duty cycle (CCR=ARR) = LEDs off (OE always HIGH)
	//
	// For brightness 0-10:
	// - Brightness 0: LEDs off (100% duty)
	// - Brightness 10: Maximum brightness (0% duty)
	// - Brightness 5: 50% brightness (50% duty)

	uint32_t duty_cycle;

	if (brightness == 0) {
		// Turn off LEDs completely
		duty_cycle = PWM_PERIOD; // 100% duty = OE always HIGH
	} else {
		// Invert brightness: higher brightness = lower duty cycle
		// Map brightness 1-10 to duty cycle 90%-0%
		duty_cycle = PWM_PERIOD - ((brightness * PWM_PERIOD) / MAX_BRIGHTNESS);
	}

	// Set PWM duty cycle on the configured channel
	__HAL_TIM_SET_COMPARE(me->htim, me->tim_channel, duty_cycle);

	log_message(tag, LOG_DEBUG, "Brightness set to %d (PWM duty: %lu%%)",
	            brightness, (duty_cycle * 100) / PWM_PERIOD);
}

void SN74HC595_update(SN74HC595_t * const me, uint16_t data, uint8_t brightness) {
	// Update brightness first (affects display immediately)
	SN74HC595_set_brightness(me, brightness);

	// Then update data pattern
	SN74HC595_write(me, data);

	log_message(tag, LOG_DEBUG, "Updated - Data: 0x%04X, Brightness: %d", data, brightness);
}

void SN74HC595_clear(SN74HC595_t * const me) {
	// Method 1: Using SRCLR pin (hardware clear)
	// This is faster but clears internal registers
	HAL_GPIO_WritePin(me->ser_clr_port, me->ser_clr_pin, GPIO_PIN_RESET);
	__NOP();
	__NOP();
	__NOP();
	HAL_GPIO_WritePin(me->ser_clr_port, me->ser_clr_pin, GPIO_PIN_SET);

	// Latch the cleared state to outputs
	pulse_latch(me->rclk_port, me->rclk_pin);

	me->current_data = 0x0000;

	log_message(tag, LOG_DEBUG, "Shift register cleared");
}

void SN74HC595_disable_output(SN74HC595_t * const me) {
	// Turn off all LEDs by setting OE to HIGH (100% duty cycle)
	__HAL_TIM_SET_COMPARE(me->htim, me->tim_channel, PWM_PERIOD);

	log_message(tag, LOG_DEBUG, "Output disabled");
}

void SN74HC595_enable_output(SN74HC595_t * const me) {
	// Restore previous brightness
	SN74HC595_set_brightness(me, me->current_brightness);

	log_message(tag, LOG_DEBUG, "Output enabled - Brightness: %d", me->current_brightness);
}

