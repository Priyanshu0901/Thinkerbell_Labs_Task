/*
 *  @file SN74HC595.h
 *
 *  Created on: 11-Jan-2026
 *      Author: Priyanshu Roy
 */

#ifndef SN74HC595_H_
#define SN74HC595_H_

#include "main.h"

typedef struct{
  //gpio_num_t oe_pin;
//  gpio_num_t rclk_pin;
//  gpio_num_t ser_data_pin;
//  gpio_num_t ser_clk_pin;
//  gpio_num_t ser_clr_pin;
}SN74HC595_t;

void SN74HC595_ctor(SN74HC595_t * const me);
void SN74HC595_write(SN74HC595_t * const me,uint16_t data);

#endif /* SN74HC595_H_ */
