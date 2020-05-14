
#ifndef LED_H_
#define LED_H_

#include "PLATFORM.h"
#include "Timer.hpp"

class Led
{
private:
public:
    static void Init();
    static void Blink(uint8_t nr, uint16_t delay);
    static void fadeAnimation();
    static void On();
    static void Off();
};

#endif /* TIMER_H_ */
