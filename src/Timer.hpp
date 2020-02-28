
#ifndef TIMER_H_
#define TIMER_H_

#include "PLATFORM.h"

class Timer
{
private:
    static uint32_t millis;
public:
    static void Init();
    static uint32_t getMillis();
    static void count();
};

void timer_init();

#endif /* TIMER_H_ */
