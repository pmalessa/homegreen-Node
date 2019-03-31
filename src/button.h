/*
 * BUTTON.h
 *
 *  Created on: 04.01.2019
 *      Author: pmale
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#define BUT_MINUS_INT PCINT0
#define BUT_PLUS_INT PCINT2
#define BUT_SET_INT PCINT1
#define BUT_MAN_INT PCINT3



typedef enum{
	BUTTON_NO_PRESS=0,
	BUTTON_SHORT_PRESS=2,	//100ms
	BUTTON_LONG_PRESS=16,	//800ms
	BUTTON_LONG_PRESSING,
	BUTTON_PRESSING
}button_press;

typedef enum{
	BUTTON_MINUS = PA0,
	BUTTON_SET = PA1,
	BUTTON_PLUS = PA2,
	BUTTON_MAN = PA3,
	NUM_BUTTONS
}button_t;

typedef struct{
	uint8_t pressed;
	uint8_t duration;
}button_struct;

void button_init();
void button_setPCCallback(void (*func)(void));
button_press button_isPressed(button_t but);
uint8_t button_anyPressed();
void button_clear();
void button_SyncTask(); //50ms




#endif /* BUTTON_H_ */
