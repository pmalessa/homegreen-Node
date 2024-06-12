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
    static void Wakeup();
    static void Sleep();
    static bool isPowerConnected();
    static bool isCapLow();
    static bool isCapFull();
    static void setInputPower(uint8_t state);
    static void setLoad(uint8_t state);
    static void run();
private:
    static uint16_t adc2vol()
    {
        currentCapVoltage = ADC;
	    uint32_t result = 1125300L / currentCapVoltage; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
	    return result;
    }

    #define CHANNEL_1V1	0b1110
    #define POWER_LOWVOLTAGE 3100	//Low Voltage Threshold in mV
    #define POWER_CAPFULL 4600	//Cap Full High Threshold in mV

    static uint16_t LoadCounter;
    static uint16_t currentCapVoltage;
    static uint8_t adcStable;
    static DeltaTimer powerTimer;
    static uint8_t powerPinIntegrator;
    static bool powerPinState;

    //100ms debouncing
    #define MAXIMUM	10

};

#endif /* POWER_H_ */
