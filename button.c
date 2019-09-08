/*
 * BUTTON.c
 *
 *  Created on: 04.01.2019
 *      Author: pmale
 */
#include "PLATFORM.h"

#include "button.h"

button_struct button[5];
volatile uint8_t button_any_pressed;

void button_dummy_callback()
{
}
void (*button_callback)(void) = &button_dummy_callback;

void button_init()
{
	PCICR = (1<<PCIE2);				//Enable Interrupt Handler for Pins 23..16
	PCMSK2 |= (1<<BUT_MINUS_INT);	//Enable Interrupt on Button Pin
	PCMSK2 |= (1<<BUT_PLUS_INT);
	PCMSK2 |= (1<<BUT_SET_INT);
	PCMSK2 |= (1<<BUT_MAN_INT);

	BUTTON_DDR &= ~(_BV(BUTTON_MINUS));			//Set Button as Input
	BUTTON_DDR &= ~(_BV(BUTTON_PLUS));
	BUTTON_DDR &= ~(_BV(BUTTON_SET));
	BUTTON_DDR &= ~(_BV(BUTTON_MAN));
}

void button_setPCCallback(void (*func)(void))	//set pin change callback function
{
	button_callback = func;
}

void button_clear()
{
	for(uint8_t i=0;i<4;i++)
	{
		button[i].pressed = 0;
		button[i].duration = 0;
	}
}

uint8_t button_anyPressed()
{
	return button_any_pressed;
}

void button_SyncTask() //50ms
{
	if(BUTTON_PIN & (1 << BUTTON_MINUS))
	{
		if(button[BUTTON_MINUS].pressed == 0)	//if new press
		{
			button[BUTTON_MINUS].pressed = 1;
			button[BUTTON_MINUS].duration = 0;
		}
		button[BUTTON_MINUS].duration++;
	}
	else
	{
		button[BUTTON_MINUS].pressed = 0;
	}
	if(BUTTON_PIN & (1 << BUTTON_SET))
	{
		if(button[BUTTON_SET].pressed == 0)	//if new press
		{
			button[BUTTON_SET].pressed = 1;
			button[BUTTON_SET].duration = 0;
		}
		button[BUTTON_SET].duration++;
	}
	else
	{
		button[BUTTON_SET].pressed = 0;
	}
	if(BUTTON_PIN & (1 << BUTTON_PLUS))
	{
		if(button[BUTTON_PLUS].pressed == 0)	//if new press
		{
			button[BUTTON_PLUS].pressed = 1;
			button[BUTTON_PLUS].duration = 0;
		}
		button[BUTTON_PLUS].duration++;
	}
	else
	{
		button[BUTTON_PLUS].pressed = 0;
	}
	if(BUTTON_PIN & (1 << BUTTON_MAN))
	{
		if(button[BUTTON_MAN].pressed == 0)	//if new press
		{
			button[BUTTON_MAN].pressed = 1;
			button[BUTTON_MAN].duration = 0;
		}
		button[BUTTON_MAN].duration++;
	}
	else
	{
		button[BUTTON_MAN].pressed = 0;
	}
}

button_press button_isPressed(button_t but)
{
	if(button[but].pressed)
	{
		if(button[but].duration > BUTTON_LONG_PRESS)
		{
			button[but].duration = 0;
			return BUTTON_LONG_PRESSING;
		}
		else
		{
			return BUTTON_PRESSING;
		}
	}
	else
	{
		if(button[but].duration > BUTTON_SHORT_PRESS)
		{
			button[but].duration = 0;
			return BUTTON_SHORT_PRESS;
		}
		else if(button[but].duration > BUTTON_LONG_PRESS)
		{
			button[but].duration = 0;
			return BUTTON_LONG_PRESS;
		}
		else
		{
			return BUTTON_NO_PRESS;
		}
	}
}

ISR(PCINT2_vect)	//Pin change on Button Pin
{
	button_any_pressed = 1;
	button_callback();
	//wake up
}
