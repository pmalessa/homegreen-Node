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
	PUMP_TEST_DDR |= _BV(PUMP_TEST_PIN);		//Set Pump Test Pin as Output
	PUMP_SIG_DDR &= ~_BV(PUMP_SIG_PIN);			//Set Pump Sig Pin as Input
	PUMP_SIG_PORT |= _BV(PUMP_SIG_PIN);			//turn on Pump Sig Pin Pullup

	PUMP_TEST_PORT &= ~_BV(PUMP_TEST_PIN);		//turn off Pump Test Pin
	PUMP_PORT &= ~_BV(PUMP_PIN);				//turn off Pump Pin
	pumpCounter = 0;
	pumpTimer.setTimeStep(1000); 				//1 second
	currentPump = PUMP_1;

    USB_OUT_P_DDR |= _BV(USB_OUT_P_PIN);        //USB1 Output
    USB_OUT_M_DDR |= _BV(USB_OUT_M_PIN);        //USB2 Output
    USB_OUT_P_PORT &= ~_BV(USB_OUT_P_PIN);      //set low
    USB_OUT_M_PORT &= ~_BV(USB_OUT_M_PIN);      //set low

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
			PUMP_PORT &= ~_BV(PUMP_PIN);	//turn off
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

    USB_OUT_P_DDR &= ~_BV(USB_OUT_P_PIN);                //USBP Input
    USB_OUT_M_DDR &= ~_BV(USB_OUT_M_PIN);                //USB2 Input
    USB_OUT_P_PORT &= ~_BV(USB_OUT_P_PIN);                    //No PullUp
    USB_OUT_M_PORT &= ~_BV(USB_OUT_M_PIN);                    //No PullUp
    _delay_ms(10);

    for(uint8_t i=0;i<10;i++)    //check pin 10 times to ignore floating pin
    {
		if(USB_OUT_P_PINREG & _BV(USB_OUT_P_PIN))
		{
			usb1_cnt++;
		}
		if(USB_OUT_M_PINREG & _BV(USB_OUT_M_PIN))
		{
			usb2_cnt++;
		}
        _delay_ms(1);
    }
	PUMP_PORT &= ~_BV(PUMP_PIN);	//turn off
    USB_OUT_P_DDR |= _BV(USB_OUT_P_PIN);                //USBP Output
    USB_OUT_M_DDR |= _BV(USB_OUT_M_PIN);                //USB2 Output
    USB_OUT_P_PORT &= ~_BV(USB_OUT_P_PIN);                //set low
    USB_OUT_M_PORT &= ~_BV(USB_OUT_M_PIN);                //set low

    if(usb1_cnt > 8 && usb2_cnt < 2)    //Hub Detection, High on USB+, Low on USB-
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Pump::isPumpConnected()
{
	PUMP_PORT &= ~_BV(PUMP_PIN);	//turn off
	PUMP_TEST_PORT |= _BV(PUMP_TEST_PIN);	//turn on test current limiter

	_delay_ms(10);	//wait 100ms

	if(!(PUMP_SIG_PINREG & _BV(PUMP_SIG_PIN)))	//check if current limit reached, if pin low
	{
		PUMP_PORT |= _BV(PUMP_PIN);	//turn on
		PUMP_TEST_PORT &= ~_BV(PUMP_TEST_PIN);	//turn off
		return true;
	}
	else
	{
		PUMP_PORT |= _BV(PUMP_PIN);	//turn on
		PUMP_TEST_PORT &= ~_BV(PUMP_TEST_PIN);	//turn off
		return false;
	}
}

void Pump::setCurrentPump(uint8_t pumpID)
{
	switch (pumpID) {
		case PUMP_1:
			currentPump = PUMP_1;
		    USB_OUT_P_PORT &= ~_BV(USB_OUT_P_PIN);                //0 0 -> pump 1
		    USB_OUT_M_PORT &= ~_BV(USB_OUT_M_PIN);
			break;
		case PUMP_2:
			currentPump = PUMP_2;
		    USB_OUT_P_PORT |= _BV(USB_OUT_P_PIN);                //1 1 -> pump 2
		    USB_OUT_M_PORT |= _BV(USB_OUT_M_PIN);
			break;
		case PUMP_3:
			currentPump = PUMP_3;
		    USB_OUT_P_PORT &= ~_BV(USB_OUT_P_PIN);                //0 1 -> pump 3
		    USB_OUT_M_PORT |= _BV(USB_OUT_M_PIN);
			break;
	}
}
