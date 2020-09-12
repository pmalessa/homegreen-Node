
#ifndef LED_H_
#define LED_H_

#include "PLATFORM.h"
#include "Timer.hpp"

#define LED_RED 1
#define LED_GREEN 2
#define LED_BTN 3

class Led
{
private:
public:
    static void Init();
    static void Blink(uint8_t ledID, uint8_t nr, uint16_t delay);
    static void On(uint8_t ledID);
    static void Off(uint8_t ledID);
};

#endif /* TIMER_H_ */
