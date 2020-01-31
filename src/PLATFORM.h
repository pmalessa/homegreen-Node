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

#define BUILD_DATE 20190817

#define BUZZER_DDR DDRB
#define BUZZER_PORT PORTB
#define BUZZER_PIN PB1

#define PWR_IN_DDR DDRC
#define PWR_IN_PORT PORTC
#define PWR_IN_PIN PC3

#define PWR_LOAD_DDR DDRB
#define PWR_LOAD_PORT PORTB
#define PWR_LOAD_PIN PB2

#define PWR_5V_DDR DDRC
#define PWR_5V_PORT PORTC
#define PWR_5V_PIN PC0
#define PWR_5V_PINREG PINC

//#define SLEEP_INTERVAL_S 4	//4000ms = 4s
#define DISPLAY_TIMEOUT_S 5 //5sec

typedef uint8_t bool;

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

#endif /* PLATFORM_H_ */
