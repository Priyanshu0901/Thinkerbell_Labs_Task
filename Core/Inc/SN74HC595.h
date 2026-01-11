/*
 *  @file SN74HC595.h
 *
 *  Created on: 11-Jan-2026
 *      Author: Priyanshu Roy
 */

#ifndef SN74HC595_H_
#define SN74HC595_H_

#include "main.h"
#include "tim.h"

typedef struct {
	// Data and Clock pins
	GPIO_TypeDef *ser_data_port;
	uint16_t ser_data_pin;

	GPIO_TypeDef *ser_clk_port;
	uint16_t ser_clk_pin;

	// Register Clock (latch)
	GPIO_TypeDef *rclk_port;
	uint16_t rclk_pin;

	// Clear pin
	GPIO_TypeDef *ser_clr_port;
	uint16_t ser_clr_pin;

	// Timer handle for PWM (brightness control on OE pin)
	TIM_HandleTypeDef *htim;
	uint32_t tim_channel;

	// Current state
	uint16_t current_data;
	uint8_t current_brightness;
} SN74HC595_t;

// Constructor - Initialize the shift register
void SN74HC595_ctor(SN74HC595_t * const me,
                    GPIO_TypeDef *data_port, uint16_t data_pin,
                    GPIO_TypeDef *clk_port, uint16_t clk_pin,
                    GPIO_TypeDef *rclk_port, uint16_t rclk_pin,
                    GPIO_TypeDef *clr_port, uint16_t clr_pin,
                    TIM_HandleTypeDef *htim, uint32_t tim_channel);

// Write 16-bit data to shift register
void SN74HC595_write(SN74HC595_t * const me, uint16_t data);

// Set brightness (0-10) via PWM duty cycle
void SN74HC595_set_brightness(SN74HC595_t * const me, uint8_t brightness);

// Write data and set brightness in one call
void SN74HC595_update(SN74HC595_t * const me, uint16_t data, uint8_t brightness);

// Clear all outputs (set to 0)
void SN74HC595_clear(SN74HC595_t * const me);

// Turn off all LEDs (via OE pin - 100% duty = LEDs off since OE is active low)
void SN74HC595_disable_output(SN74HC595_t * const me);

// Turn on LEDs (restore brightness)
void SN74HC595_enable_output(SN74HC595_t * const me);

#endif /* SN74HC595_H_ */
