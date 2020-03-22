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
    static void setCurrentPump(uint8_t pumpID);
    static uint8_t getCurrentPump();
private:
    static DeltaTimer pumpTimer;
    static uint16_t pumpCounter;
    static uint8_t currentPump;

    #define PUMP_DDR DDRA
    #define PUMP_PORT PORTA
    #define PUMP_PIN PA1

    #define PUMP_SIG_DDR DDRC
    #define PUMP_SIG_PORT PORTC
    #define PUMP_SIG_PIN PC7

    #define PUMP_TEST_DDR DDRC
    #define PUMP_TEST_PORT PORTC
    #define PUMP_TEST_PIN PC0

    #define USBM_DDR DDRC
    #define USBM_PORT PORTC
    #define USBM_PIN PC1
    #define USBM_PINREG PINC

    #define USBP_DDR DDRD
    #define USBP_PORT PORTD
    #define USBP_PIN PD0
    #define USBP_PINREG PIND

    #define PUMP_1 0
    #define PUMP_2 1
    #define PUMP_3 2


};
#endif /* PUMP_H_ */
