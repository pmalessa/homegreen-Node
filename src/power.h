/*
 * power.h
 *
 *  Created on: 26.02.2019
 *      Author: pmale
 */

#ifndef POWER_H_
#define POWER_H_

#define CHANNEL_1V1	0b1101

#define LOWVOLTAGE 2000	//Low Voltage Threshold in mV
#define POWER_THRESHOLD 3000

void power_init();
void power_setCallback(void (*func)(void));	//set pin change callback function
uint8_t power_isPowerConnected();
void power_setInputPower(uint8_t state);

void power_SyncTask();	//every second

#endif /* POWER_H_ */
