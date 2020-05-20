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

#define BUILD_DAY 20
#define BUILD_MONTH 5
#define BUILD_YEAR 20
#define BUILD_VERSION_MAJOR 1
#define BUILD_VERSION_MINOR 4
#define BUILD_VERSION_PATCH 0

#define DEBUG1_DDR DDRD
#define DEBUG1_PORT PORTD
#define DEBUG1_PIN PD7

#define EN_PB_DDR DDRC
#define EN_PB_PORT PORTC
#define EN_PB_PIN PC2

#define EN_LOAD_DDR DDRC
#define EN_LOAD_PORT PORTC
#define EN_LOAD_PIN PC3

#define CHK_5V_DDR DDRA
#define CHK_5V_PORT PORTA
#define CHK_5V_PIN PA0
#define CHK_5V_PINREG PINA

#define TOUCH_PWR_DDR DDRA
#define TOUCH_PWR_PORT PORTA
#define TOUCH_PWR_PIN PA2

#define I2C_SCL_DDR DDRC
#define I2C_SCL_PORT PORTC
#define I2C_SCL_PIN PC5

#define I2C_SDA_DDR DDRC
#define I2C_SDA_PORT PORTC
#define I2C_SDA_PIN PC4

#define EN_CLK_DDR DDRD
#define EN_CLK_PORT PORTD
#define EN_CLK_PIN PD0

#define LED_DDR DDRB
#define LED_PORT PORTB
#define LED_PIN PB2

#define PUMP_DDR DDRA
#define PUMP_PORT PORTA
#define PUMP_PIN PA1

#define PUMP_SIG_DDR DDRC
#define PUMP_SIG_PORT PORTC
#define PUMP_SIG_PIN PC7
#define PUMP_SIG_PINREG PINC

#define PUMP_TEST_DDR DDRC
#define PUMP_TEST_PORT PORTC
#define PUMP_TEST_PIN PC0

#define USB_OUT_M_DDR DDRC
#define USB_OUT_M_PORT PORTC
#define USB_OUT_M_PIN PC1
#define USB_OUT_M_PINREG PINC

#define USB_OUT_P_DDR DDRB
#define USB_OUT_P_PORT PORTB
#define USB_OUT_P_PIN PB0
#define USB_OUT_P_PINREG PINB

#define USB_IN_M_DDR DDRB
#define USB_IN_M_PORT PORTB
#define USB_IN_M_PIN PB7
#define USB_IN_M_PINREG PINB

#define USB_IN_P_DDR DDRB
#define USB_IN_P_PORT PORTB
#define USB_IN_P_PIN PB6
#define USB_IN_P_PINREG PINB

#define TM_OUT          PORTD
#define TM_IN           PIND
#define TM_DDR          DDRD
#define TM_BIT_CLK      _BV(PD6)
#define TM_BIT_DAT      _BV(PD5)

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
