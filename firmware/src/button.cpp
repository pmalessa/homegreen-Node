
#include "button.hpp"

Button::button_struct Button::button[5] = {0};
bool Button::anyPressed = 0;
DeltaTimer Button::buttonTimer;

void (*button_callback)(void) = nullptr;

void Button::Init()
{
	PCICR = _BV(PCIE2);				//Enable Interrupt Handler for Pins 23..16
	PCMSK2 |= _BV(BUT_MINUS_INT);	//Enable Interrupt on Button Pin
	PCMSK2 |= _BV(BUT_PLUS_INT);
	PCMSK2 |= _BV(BUT_SET_INT);
	PCMSK2 |= _BV(BUT_MAN_INT);

	BUTTON_DDR &= ~_BV(BUTTON_MINUS);			//Set Button as Input
	BUTTON_DDR &= ~_BV(BUTTON_PLUS);
	BUTTON_DDR &= ~_BV(BUTTON_SET);
	BUTTON_DDR &= ~_BV(BUTTON_MAN);

	TOUCH_PWR_DDR |= _BV(TOUCH_PWR_PIN);
	TOUCH_PWR_PORT |= _BV(TOUCH_PWR_PIN);

	buttonTimer.setTimeStep(10);
}

void Button::DeInit()
{
	PCICR &= ~ _BV(PCIE2);	//Disable Interrupts
	TOUCH_PWR_PORT &= ~ _BV(TOUCH_PWR_PIN); //Disable Touch Power
}

void Button::SetCallback(void (*func)(void))	//set pin change callback function
{
	button_callback = func;
}

void Button::Clear()
{
	for(uint8_t i=0;i<5;i++)
	{
		button[i].pressed = 0;
		button[i].duration = 0;
	}
	anyPressed = false;
}

void Button::clearOtherThan(button_t but)
{
	for(uint8_t i=0;i<5;i++)
	{
		if(i != but)
		{
			button[i].pressed = 0;
			button[i].duration = 0;
		}
	}
}

bool Button::isAnyPressed()
{
	uint8_t tmp = anyPressed;
	anyPressed = false;
	return tmp;
}

void Button::run() 
{
	if(buttonTimer.isTimeUp()) //10ms
	{
		for(uint8_t i=1;i<5;i++)	//1..4 -> PD1..PD4
		{
			if(BUTTON_PIN & _BV(i))
			{
				if(button[i].pressed == 0)	//if new press
				{
					button[i].pressed = 1;
					button[i].duration = 0;
				}
				button[i].duration++;
			}
			else
			{
				button[i].pressed = 0;
			}
		}
	}
}

/*
	- button still pressed and duration > LONG PRESS THRESHOLD -> Long Press
	- button released and last Press > SHORT PRESS THRESHOLD -> Short Press
	- else no press
*/
Button::button_press Button::isPressed(Button::button_t but)
{
	if(button[but].pressed)
	{
		if(button[but].duration > BUTTON_LONGPRESS_THRESHOLD)
		{
			return BUTTON_LONG_PRESS;
		}
		else
		{
			return BUTTON_NO_PRESS;
		}
		
	}
	else
	{
		if(button[but].duration > BUTTON_SHORTPRESS_THRESHOLD)
		{
			button[but].duration = 0;
			return BUTTON_SHORT_PRESS;
		}
		else
		{
			button[but].duration = 0;
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
}
