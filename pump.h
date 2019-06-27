/*
 * PUMP.h
 *
 *  Created on: 17.01.2019
 *      Author: pmale
 */

#ifndef PUMP_H_
#define PUMP_H_

#define PUMP_DDR DDRA
#define PUMP_PORT PORTA
#define PUMP_PIN PA1

#define PUMP_SIG_DDR DDRC
#define PUMP_SIG_PORT PORTC
#define PUMP_SIG_PIN PC7

#define PUMP_TEST_DDR DDRA
#define PUMP_TEST_PORT PORTA
#define PUMP_TEST_PIN PA0

void pump_init();

void pump_vSyncTask();	//10ms

void pump_enable(uint16_t val);

void pump_disable();

uint16_t pump_getCountdown();

void pump_setPower(uint8_t pow);

uint8_t pump_getPower();


#endif /* PUMP_H_ */
