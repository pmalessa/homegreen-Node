/*
 * PUMP.h
 *
 *  Created on: 17.01.2019
 *      Author: pmale
 */

#ifndef PUMP_H_
#define PUMP_H_

#include "PLATFORM.h"
#include "DeltaTimer.hpp"

class Pump
{

public:
    static void Init();
    static void Start();
    static void Stop();
    static void setCountdown(uint16_t sec);
    static uint16_t getCountdown();
    static void Increment();
    static void Decrement();
    static void run();
    static bool isHubConnected();
    static bool isPumpConnected();
    static void setCurrentPump(uint8_t pumpID);
    static uint8_t getCurrentPump();
private:
    static DeltaTimer pumpTimer;
    static uint16_t pumpCounter;
    static uint8_t currentPump;

    #define PUMP_1 0
    #define PUMP_2 1
    #define PUMP_3 2


};
#endif /* PUMP_H_ */
