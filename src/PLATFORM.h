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

#define BUILD_DAY 05
#define BUILD_MONTH 07
#define BUILD_YEAR 22
#define BUILD_VERSION_MAJOR 1
#define BUILD_VERSION_MINOR 4
#define BUILD_VERSION_PATCH 7

#ifdef HG_BASIC_HW_1_4_4

#define FEATURE_PUMP_STRENGTH

#define CHK_5V_DDR DDRA
#define CHK_5V_PORT PORTA
#define CHK_5V_PIN PA0
#define CHK_5V_PINREG PINA

#define PUMP_DDR DDRA
#define PUMP_PORT PORTA
#define PUMP_PIN PA1

#define TOUCH_PWR_DDR DDRA
#define TOUCH_PWR_PORT PORTA
#define TOUCH_PWR_PIN PA2

#define USB_OUT_P_DDR DDRB
#define USB_OUT_P_PORT PORTB
#define USB_OUT_P_PIN PB0
#define USB_OUT_P_PINREG PINB

#define EN_CLK_DDR DDRB
#define EN_CLK_PORT PORTB
#define EN_CLK_PIN PB1

#define LED_GREEN_DDR DDRB
#define LED_GREEN_PORT PORTB
#define LED_GREEN_PIN PB2

#define LED_BTN_DDR DDRB
#define LED_BTN_PORT PORTB
#define LED_BTN_PIN PB7

#define PUMP_VOL_DDR DDRC
#define PUMP_VOL_PORT PORTC
#define PUMP_VOL_PIN PC0

#define USB_OUT_M_DDR DDRC
#define USB_OUT_M_PORT PORTC
#define USB_OUT_M_PIN PC1
#define USB_OUT_M_PINREG PINC

#define EN_PB_DDR DDRC
#define EN_PB_PORT PORTC
#define EN_PB_PIN PC2

#define PUMP_CUR_DDR DDRC
#define PUMP_CUR_PORT PORTC
#define PUMP_CUR_PIN PC3
#define PUMP_CUR_PINREG PINC

#define I2C_SDA_DDR DDRC
#define I2C_SDA_PORT PORTC
#define I2C_SDA_PIN PC4

#define I2C_SCL_DDR DDRC
#define I2C_SCL_PORT PORTC
#define I2C_SCL_PIN PC5

#define EN_LOAD_DDR DDRD
#define EN_LOAD_PORT PORTD
#define EN_LOAD_PIN PD0

#define TM_OUT          PORTD
#define TM_IN           PIND
#define TM_DDR          DDRD
#define TM_BIT_DAT      _BV(PD5)
#define TM_BIT_CLK      _BV(PD6)

#define LED_RED_DDR DDRD
#define LED_RED_PORT PORTD
#define LED_RED_PIN PD7

#elif defined(HG_BASIC_HW_1_4_3)
#define DEBUG1_DDR DDRC
#define DEBUG1_PORT PORTC
#define DEBUG1_PIN PC0

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

#define EN_CLK_DDR DDRB
#define EN_CLK_PORT PORTB
#define EN_CLK_PIN PB1

#define LED_RED_DDR DDRD
#define LED_RED_PORT PORTD
#define LED_RED_PIN PD7

#define LED_GREEN_DDR DDRB
#define LED_GREEN_PORT PORTB
#define LED_GREEN_PIN PB2

#define LED_BTN_DDR DDRB
#define LED_BTN_PORT PORTB
#define LED_BTN_PIN PB7

#define PUMP_DDR DDRA
#define PUMP_PORT PORTA
#define PUMP_PIN PA1

#define USB_OUT_M_DDR DDRC
#define USB_OUT_M_PORT PORTC
#define USB_OUT_M_PIN PC1
#define USB_OUT_M_PINREG PINC

#define USB_OUT_P_DDR DDRB
#define USB_OUT_P_PORT PORTB
#define USB_OUT_P_PIN PB0
#define USB_OUT_P_PINREG PINB

#define USB_IN_SW_DDR DDRC
#define USB_IN_SW_PORT PORTC
#define USB_IN_SW_PIN PC7

#define TM_OUT          PORTD
#define TM_IN           PIND
#define TM_DDR          DDRD
#define TM_BIT_CLK      _BV(PD6)
#define TM_BIT_DAT      _BV(PD5)
#endif

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
	DIGIT_COUNTDOWN,
}digit_t;

#define UNUSED(x) (void)(x)

/* Macros to access bytes within words and words within longs */
#define LOW_BYTE(x)     ((uint8_t)((x)&0xFF))
#define HIGH_BYTE(x)    ((uint8_t)(((x)>>8)&0xFF))
#define LOW_WORD(x)     ((uint16_t)((x)&0xFFFF))
#define HIGH_WORD(x)    ((uint16_t)(((x)>>16)&0xFFFF))

#endif /* PLATFORM_H_ */
