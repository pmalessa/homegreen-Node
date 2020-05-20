/*
 * BUTTON.h
 *
 *  Created on: 04.01.2019
 *      Author: pmale
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "PLATFORM.h"
#include "DeltaTimer.hpp"

class Button
{
public:
	typedef enum{
		BUTTON_NO_PRESS=0,
		BUTTON_SHORT_PRESS,
		BUTTON_LONG_PRESS
	}button_press;

	typedef enum{
		BUTTON_PLUS = PD1,
		BUTTON_SET = PD2,
		BUTTON_MINUS = PD3,
		BUTTON_MAN = PD4,
		NUM_BUTTONS
	}button_t;

	static void Init();
	static void SetCallback(void (*func)(void));
	static button_press isPressed(button_t but);
	static bool isAnyPressed();
	static void Clear();
	static void run();
	static void clearOtherThan(button_t but);

	static void setAnyPressed();

private:
	#define BUT_MINUS_INT PCINT19
	#define BUT_PLUS_INT PCINT17
	#define BUT_SET_INT PCINT18
	#define BUT_MAN_INT PCINT20

	#define BUTTON_DDR DDRD
	#define BUTTON_PIN PIND

	#define BUTTON_SHORTPRESS_THRESHOLD 5 //50ms
	#define BUTTON_LONGPRESS_THRESHOLD 80 //800ms

	typedef struct{
		uint8_t pressed;
		uint16_t duration;
	}button_struct;

	static button_struct button[5];
	static bool anyPressed;
	static DeltaTimer buttonTimer;

};



#endif /* BUTTON_H_ */
