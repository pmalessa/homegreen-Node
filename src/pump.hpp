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
    static void Enable(uint16_t sec);
    static void Disable();
    static uint16_t getCountdown();
    static void run();
private:
    static DeltaTimer pumpTimer;
    static uint16_t pumpCounter;
    #define PUMP_DDR DDRA
    #define PUMP_PORT PORTA
    #define PUMP_PIN PA1

    #define PUMP_SIG_DDR DDRA
    #define PUMP_SIG_PORT PORTA
    #define PUMP_SIG_PIN PA0

    #define PUMP_TEST_DDR DDRC
    #define PUMP_TEST_PORT PORTC
    #define PUMP_TEST_PIN PC7
};
#endif /* PUMP_H_ */
