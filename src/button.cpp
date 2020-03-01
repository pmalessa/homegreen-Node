
#include "button.hpp"

Button::button_struct Button::button[5] = {0};
bool Button::anyPressed = 0;
DeltaTimer Button::buttonTimer;

void (*button_callback)(void) = nullptr;

void Button::Init()
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

	buttonTimer.setTimeStep(50);
}

void Button::SetCallback(void (*func)(void))	//set pin change callback function
{
	button_callback = func;
}

void Button::Clear()
{
	for(uint8_t i=0;i<4;i++)
	{
		button[i].pressed = 0;
		button[i].duration = 0;
	}
}

bool Button::isAnyPressed()
{
	return anyPressed;
}

void Button::run() 
{
	if(buttonTimer.isTimeUp()) //50ms
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
}

Button::button_press Button::isPressed(Button::button_t but)
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

void Button::setAnyPressed()
{
	anyPressed = true;
}

ISR(PCINT2_vect)	//Pin change on Button Pin
{
	Button::setAnyPressed();
	if(button_callback != nullptr) button_callback();
	//wake up
}
