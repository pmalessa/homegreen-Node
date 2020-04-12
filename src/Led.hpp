
#ifndef LED_H_
#define LED_H_

#include "PLATFORM.h"

class Led
{
private:
public:
    static void Init();
    static void Blink(uint8_t nr, uint16_t delay);
    static void On();
    static void Off();

    #define LED_DDR DDRC
    #define LED_PORT PORTC
    #define LED_PIN PC2
};

#endif /* TIMER_H_ */
