/*
 * display.h
 *
 *  Created on: 29.01.2019
 *      Author: pmale
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "PLATFORM.h"
#include "DeltaTimer.hpp"
extern "C"
{
  #include "driver/tm1637.h"
}


class Display
{
public:
    #define REFRESH_RATE 100 //ms
    #define DEC_DOT 0x80

    typedef enum{
        ANIMATION_NONE,
	    ANIMATION_BOOT,
	    ANIMATION_WAKE,
	    ANIMATION_CHARGE,
        ANIMATION_PUMP,
        ANIMATION_FADE
    }animation_t;

    static void Init();
    static void DeInit();
    static void Clear();
    static void SetValue(digit_t digit, uint16_t val); //val = (0.1 .. 1.0,1.1 .. 10.0,11.0 .. 99.0) * 10 = 1..990
    static void SetNegValue(uint8_t position, int16_t val);
    static void Set4DigValue(uint8_t position, uint32_t val);
    static void SetByte(uint8_t pos, uint8_t byte);
    static void SetBrightness(uint8_t val); //0..7
    static void EnableBlinking(digit_t digit);
    static void DisableBlinking();
    static void StartAnimation(animation_t animation);
    static void StopAnimation();
    static bool IsAnimationDone();
    static void Draw();
    static void ResetTimeout();
    static bool IsTimeout();
    static uint8_t numToByte(uint8_t num);
    static void ResetBlinkCounter();
private:
    static void setDot(uint8_t dot_pos, uint8_t val) //left to right
    {
	    if(val)
	    {
		    dotmask |= _BV(dot_pos);
	    }
	    else
	    {
		    dotmask &= ~_BV(dot_pos);
	    }
    }
    static void dsend(uint8_t pos, uint8_t byte)
    {
	    tm1637_setByte(pos,byte | (dotmask & (1 << pos) ? DEC_DOT : 0));
    }

    static animation_t currentAnimation;
    static bool isInitialized, animationDone, resetAnimation;
    static DeltaTimer displayTimer, timeoutTimer;
    static uint8_t dotmask, brightness, dig[6], blinkingEnabled ,blinkCounter;
    
};



#endif /* DISPLAY_H_ */
