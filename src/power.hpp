/*
 * power.h
 *
 *  Created on: 26.02.2019
 *      Author: pmale
 */

#ifndef POWER_H_
#define POWER_H_

#include "PLATFORM.h"
#include "DeltaTimer.hpp"

class Power
{
public:
    static void Init();
    static void DeInit();
    static bool isPowerConnected();
    static bool isPowerLow();
    static void setInputPower(uint8_t state);
    static void setLoad(uint8_t state);
    static bool isAdcStable();
    static void setGracePeriod();
    static bool isPowerLost();
    static void run();
private:
    static uint16_t measureVoltage()
    {
	    uint32_t val = 0;
	    for(uint8_t i=0;i<5;i++)
	    {
		    val +=volBuffer[i];
	    }
	    val = val / 5;
	    uint32_t result = 1125300L / val; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
	    return result;
    }

    #define CHANNEL_1V1	0b1110
    #define LOWVOLTAGE 2800	//Low Voltage Threshold in mV
    #define POWER_LOW_THRESHOLD 3400
    #define POWER_HIGH_THRESHOLD 3700

    static uint16_t LoadCounter;
    static uint16_t volBuffer[5];
    static uint8_t adcStable;
    static uint8_t gracePeriod;
    static DeltaTimer powerTimer;
};

#endif /* POWER_H_ */
