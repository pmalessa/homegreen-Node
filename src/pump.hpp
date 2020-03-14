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
    static void Increment();
    static void Decrement();
    static void run();
    static bool isHubConnected();
    static void selectPump(uint8_t pumpID);
private:
    static DeltaTimer pumpTimer;
    static uint16_t pumpCounter;
    #define PUMP_DDR DDRA
    #define PUMP_PORT PORTA
    #define PUMP_PIN PA1

    #define PUMP_SIG_DDR DDRC
    #define PUMP_SIG_PORT PORTC
    #define PUMP_SIG_PIN PC7

    #define PUMP_TEST_DDR DDRC
    #define PUMP_TEST_PORT PORTC
    #define PUMP_TEST_PIN PC0

    #define USB1_DDR DDRC
    #define USB1_PORT PORTC
    #define USB1_PIN PC1
    #define USB1_PINREG PINC

    #define USB2_DDR DDRD
    #define USB2_PORT PORTD
    #define USB2_PIN PD0
    #define USB2_PINREG PIND

    #define PUMP_1 1
    #define PUMP_2 2
    #define PUMP_3 3


};
#endif /* PUMP_H_ */
