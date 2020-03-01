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
}

void Pump::run()	//1000ms
{
	if(pumpTimer.isTimeUp())
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

void Pump::Enable(uint16_t sec)	//0.1min steps
{
	pumpCounter = sec;
}

void Pump::Disable()
{
	pumpCounter = 0;
}

uint16_t Pump::getCountdown()
{
	return pumpCounter;
}
