/*
 * PUMP.c
 *
 *  Created on: 17.01.2019
 *      Author: pmale
 */

#include "pump.hpp"

DeltaTimer Pump::pumpTimer;
uint16_t Pump::pumpCounter = 0;

void Pump::Init()
{
	PUMP_DDR |= _BV(PUMP_PIN);					//Set Pump Pin as Output
	//PUMP_TEST_DDR |= _BV(PUMP_TEST_PIN);		//Set Pump Test Pin as Output
	//PUMP_SIG_DDR &= ~(_BV(PUMP_SIG_PIN));		//Set Pump Sig Pin as Input
	
	//PUMP_TEST_PORT &= ~(_BV(PUMP_TEST_PIN));	//turn off Pump Test Pin
	PUMP_PORT &= ~(_BV(PUMP_PIN));				//turn off Pump Pin
	pumpCounter = 0;
	pumpTimer.setTimeStep(1000); //1 second

    USB1_DDR |= (_BV(USB1_PIN));                //USB1 Output
    USB2_DDR |= (_BV(USB2_PIN));                //USB2 Output
    USB1_PORT &= ~(_BV(USB1_PIN));                //set low
    USB2_PORT &= ~(_BV(USB2_PIN));                //set low

}

void Pump::run()
{
	if(pumpTimer.isTimeUp()) //1000ms
	{
		if(pumpCounter > 0)	//if enabled
		{
			PUMP_PORT |= _BV(PUMP_PIN);	//turn on
			pumpCounter--;	//countdown
		}
		else
		{
			PUMP_PORT &= ~(_BV(PUMP_PIN));	//turn off
		}
	}
}

void Pump::Enable(uint16_t sec)
{
	pumpCounter = sec;
}

void Pump::Increment()	//1min steps
{
	pumpCounter += 60;
}

void Pump::Decrement()	//1min steps
{
	if(pumpCounter <=60)
	{
		pumpCounter = 0;
	}
	else
	{
		pumpCounter -= 60;
	}
}

void Pump::Disable()
{
	pumpCounter = 0;
}

uint16_t Pump::getCountdown()
{
	return pumpCounter;
}

bool Pump::isHubConnected()
{
	uint8_t usb1_cnt = 0, usb2_cnt = 0;

    USB1_DDR &= ~(_BV(USB1_PIN));                //USB1 Input
    USB2_DDR &= ~(_BV(USB2_PIN));                //USB2 Input
    USB1_PORT |= _BV(USB1_PIN);                    //PullUp
    USB2_PORT |= _BV(USB2_PIN);                    //PullUp
    _delay_ms(10);

    for(uint8_t i=0;i<10;i++)    //check pin 10 times to ignore floating pin
    {
        usb1_cnt += (USB1_PINREG & (1 << USB1_PIN));
        usb2_cnt += (USB2_PINREG & (1 << USB2_PIN));
        _delay_ms(1);
    }
    if(usb1_cnt > 8 && usb2_cnt < 2)    //Hub Detection, High on USB+, Low on USB-
    {
        return true;
    }
    else
    {
        return false;
    }

    USB1_DDR |= (_BV(USB1_PIN));                //USB1 Output
    USB2_DDR |= (_BV(USB2_PIN));                //USB2 Output
    USB1_PORT &= ~(_BV(USB1_PIN));                //set low
    USB2_PORT &= ~(_BV(USB2_PIN));                //set low
}

void Pump::selectPump(uint8_t pumpID)
{
	switch (pumpID) {
		case PUMP_1:
		    USB1_PORT &= ~(_BV(USB1_PIN));                //1 1 -> pump 1
		    USB2_PORT &= ~(_BV(USB2_PIN));
			break;
		case PUMP_2:
		    USB1_PORT |= (_BV(USB1_PIN));                //0 0 -> pump 2
		    USB2_PORT |= (_BV(USB2_PIN));
			break;
		case PUMP_3:
		    USB1_PORT &= ~(_BV(USB1_PIN));                //0 1 -> pump 3
		    USB2_PORT |= (_BV(USB2_PIN));
			break;
	}
}
