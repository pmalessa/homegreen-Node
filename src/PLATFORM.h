/*
 * PLATFORM.h
 *
 *  Created on: 25.01.2019
 *      Author: pmale
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <avr/io.h>
#define F_CPU 1000000UL  // 1 MHz
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <stdlib.h>

#define BUILD_DATE 20190422

#define BUZZER_DDR DDRB
#define BUZZER_PORT PORTB
#define BUZZER_PIN PB1

#define PWR_IN_DDR DDRC
#define PWR_IN_PORT PORTC
#define PWR_IN_PIN PC3

#define SLEEP_INTERVAL_S 4	//4000ms = 4s
#define DISPLAY_TIMEOUT_S 15 //30sec

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
