
#ifndef TIMER_H_
#define TIMER_H_

#include "PLATFORM.h"

class Timer
{
private:
    static volatile uint32_t millis;
public:
    static void Init();
    static void Sleep();
    static void Wakeup();
    static uint32_t getMillis();
    static void count();
};

#endif /* TIMER_H_ */
