/*
 * PUMP.c
 *
 *  Created on: 17.01.2019
 *      Author: pmale
 */

#include "pump.hpp"

DeltaTimer Pump::pumpTimer;
uint16_t Pump::pumpCounter = 0;
uint8_t Pump::currentPump = PUMP_1;

void Pump::Init()
{
	PUMP_DDR |= _BV(PUMP_PIN);					//Set Pump Pin as Output
	//PUMP_TEST_DDR |= _BV(PUMP_TEST_PIN);		//Set Pump Test Pin as Output
	//PUMP_SIG_DDR &= ~(_BV(PUMP_SIG_PIN));		//Set Pump Sig Pin as Input
	
	//PUMP_TEST_PORT &= ~(_BV(PUMP_TEST_PIN));	//turn off Pump Test Pin
	PUMP_PORT &= ~(_BV(PUMP_PIN));				//turn off Pump Pin
	pumpCounter = 0;
	pumpTimer.setTimeStep(1000); //1 second
	currentPump = PUMP_1;

    USBP_DDR |= (_BV(USBP_PIN));                //USB1 Output
    USBM_DDR |= (_BV(USBM_PIN));                //USB2 Output
    USBP_PORT &= ~(_BV(USBP_PIN));                //set low
    USBM_PORT &= ~(_BV(USBM_PIN));                //set low

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

void Pump::Start()
{
	currentPump = PUMP_1;
}

void Pump::setCountdown(uint16_t sec)
{
	pumpCounter = sec;
}

void Pump::Increment()	//1min steps
{
	pumpCounter += 60;
}

void Pump::Decrement()	//1min steps
{
	if(pumpCounter >60)
	{
		pumpCounter -= 60;
	}
}

void Pump::Stop()
{
	pumpCounter = 0;
}

uint16_t Pump::getCountdown()
{
	return pumpCounter;
}

uint8_t Pump::getCurrentPump()
{
	return currentPump;
}

bool Pump::isHubConnected()
{
	uint8_t usb1_cnt = 0, usb2_cnt = 0;

	PUMP_PORT |= _BV(PUMP_PIN);	//turn on

    USBP_DDR &= ~(_BV(USBP_PIN));                //USBP Input
    USBM_DDR &= ~(_BV(USBM_PIN));                //USB2 Input
    USBP_PORT &= ~(_BV(USBP_PIN));                    //No PullUp
    USBM_PORT &= ~(_BV(USBM_PIN));                    //No PullUp
    _delay_ms(10);

    for(uint8_t i=0;i<10;i++)    //check pin 10 times to ignore floating pin
    {
		if(USBP_PINREG & (1 << USBP_PIN))
		{
			usb1_cnt++;
		}
		if(USBM_PINREG & (1 << USBM_PIN))
		{
			usb2_cnt++;
		}
        _delay_ms(1);
    }
	PUMP_PORT &= ~(_BV(PUMP_PIN));	//turn off
    USBP_DDR |= (_BV(USBP_PIN));                //USBP Output
    USBM_DDR |= (_BV(USBM_PIN));                //USB2 Output
    USBP_PORT &= ~(_BV(USBP_PIN));                //set low
    USBM_PORT &= ~(_BV(USBM_PIN));                //set low

    if(usb1_cnt > 8 && usb2_cnt < 2)    //Hub Detection, High on USB+, Low on USB-
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Pump::setCurrentPump(uint8_t pumpID)
{
	switch (pumpID) {
		case PUMP_1:
			currentPump = PUMP_1;
		    USBP_PORT &= ~(_BV(USBP_PIN));                //0 0 -> pump 1
		    USBM_PORT &= ~(_BV(USBM_PIN));
			break;
		case PUMP_2:
			currentPump = PUMP_2;
		    USBP_PORT |= (_BV(USBP_PIN));                //1 1 -> pump 2
		    USBM_PORT |= (_BV(USBM_PIN));
			break;
		case PUMP_3:
			currentPump = PUMP_3;
		    USBP_PORT &= ~(_BV(USBP_PIN));                //0 1 -> pump 3
		    USBM_PORT |= (_BV(USBM_PIN));
			break;
	}
}
