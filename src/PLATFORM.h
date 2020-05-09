/*
 * PLATFORM.h
 *
 *  Created on: 25.01.2019
 *      Author: pmale
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <stdlib.h>

#define BUILD_DATE 20200308

#define DEBUG1_DDR DDRB
#define DEBUG1_PORT PORTB
#define DEBUG1_PIN PB1

#define PWR_IN_DDR DDRC
#define PWR_IN_PORT PORTC
#define PWR_IN_PIN PC3

#define PWR_LOAD_DDR DDRD
#define PWR_LOAD_PORT PORTD
#define PWR_LOAD_PIN PD7

#define PWR_5V_DDR DDRA
#define PWR_5V_PORT PORTA
#define PWR_5V_PIN PA0
#define PWR_5V_PINREG PINA

#define LED_DDR DDRC
#define LED_PORT PORTC
#define LED_PIN PC2

#define TOUCH_PWR_DDR DDRA
#define TOUCH_PWR_PORT PORTA
#define TOUCH_PWR_PIN PA2

#define I2C_SCL_DDR DDRC
#define I2C_SCL_PORT PORTC
#define I2C_SCL_PIN PC5

#define I2C_SDA_DDR DDRC
#define I2C_SDA_PORT PORTC
#define I2C_SDA_PIN PC4

//#define SLEEP_INTERVAL_S 4	//4000ms = 4s
#define DISPLAY_TIMEOUT_S 10 //5sec

#define true 1
#define false 0

typedef struct{
	uint16_t u16interval;
	uint16_t u16duration;
	uint16_t countdown;
}config_t;

typedef enum{
	DIGIT_INTERVAL,
	DIGIT_DURATION,
	DIGIT_COUNTDOWN
}digit_t;

#define UNUSED(x) (void)(x)

#endif /* PLATFORM_H_ */
