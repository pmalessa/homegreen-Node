/*
 * power.h
 *
 *  Created on: 26.02.2019
 *      Author: pmale
 */

#ifndef POWER_H_
#define POWER_H_

#define CHANNEL_1V1	0b1110

#define LOWVOLTAGE 2500	//Low Voltage Threshold in mV
#define POWER_THRESHOLD 3700

void power_init();
uint8_t power_isPowerConnected();
uint8_t power_isPowerLow();
void power_setInputPower(uint8_t state);
uint8_t power_isAdcStable();

void power_SyncTask();	//every 10ms

#endif /* POWER_H_ */
