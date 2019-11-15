/*
 * power.h
 *
 *  Created on: 26.02.2019
 *      Author: pmale
 */

#ifndef POWER_H_
#define POWER_H_

#define CHANNEL_1V1	0b1110

#define LOWVOLTAGE 2800	//Low Voltage Threshold in mV
#define POWER_LOW_THRESHOLD 3400
#define POWER_HIGH_THRESHOLD 3700

void power_init();
uint8_t power_isPowerConnected();
uint8_t power_isPowerLow();
void power_setInputPower(uint8_t state);
void power_setLoad(uint8_t state);
uint8_t power_isAdcStable();
void power_SetGracePeriod();
uint8_t power_isPowerLost();

void power_SyncTask();	//every 10ms

#endif /* POWER_H_ */
