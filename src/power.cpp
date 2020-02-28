/*
 * power.c
 *
 *  Created on: 26.02.2019
 *      Author: pmale
 */

#include "power.hpp"

void Power::Init() {
	//enable ADC in PRR
    ADCSRA = _BV(ADEN) | _BV(ADATE)| _BV(ADPS1) | _BV(ADPS0);	//Enable, Auto Trigger, DIV8
    ADCSRB = 0;													//Free running mode
	ADMUX = CHANNEL_1V1;										//measuring 1.1V Reference Voltage
	ADMUX |= (1<<REFS0);										//using VCC Reference
	ADCSRA |= _BV(ADSC);										//start conversion
    PWR_IN_DDR |= _BV(PWR_IN_PIN);
    PWR_LOAD_DDR |= _BV(PWR_LOAD_PIN);
    PWR_5V_DDR &= ~(_BV(PWR_LOAD_PIN)); 	//digital input

    PWR_LOAD_PORT &= ~(_BV(PWR_LOAD_PIN));	//turn off load
	
	adcStable = false;
	gracePeriod = 0;
	powerTimer.setTimeStep(10); //10 ms
}

void Power::DeInit()
{
	ADCSRA = 0;	//disable ADC
	//disable ADC in PRR
}

bool Power::isAdcStable()
{
	return (adcStable == 10);	//ADC is stable once 10 measurements are buffered
}

bool Power::isPowerConnected()	//return if last measured CurVol lower than Threshold
{
	uint16_t curVol = measureVoltage();
	if(gracePeriod > 0)
	{
		return true;
	}
	if(curVol > POWER_HIGH_THRESHOLD)	//higher bound
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

bool Power::isPowerLost()	//return if last measured CurVol lower than Threshold
{
	uint16_t curVol = measureVoltage();
	if(gracePeriod > 0)
	{
		return false;
	}
	if(curVol < POWER_LOW_THRESHOLD)	//lower bound
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

bool Power::isPowerLow()	//return if last measured CurVol lower than Threshold
{
	uint16_t curVol = measureVoltage();
	if(gracePeriod > 0)
	{
		return false;
	}
	if(curVol < LOWVOLTAGE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void Power::setGracePeriod()
{
	gracePeriod = 2; //2 seconds
}

void Power::setInputPower(uint8_t state)
{
	if (state == 1)
	{
		PWR_IN_PORT &= ~(_BV(PWR_IN_PIN)); 		//turn on PB
	}
	else
	{
		PWR_IN_PORT |= (_BV(PWR_IN_PIN));	//turn off PB
	}
}

void Power::setLoad(uint8_t state)
{
	if(state == 1)
	{
		PWR_LOAD_PORT |= _BV(PWR_LOAD_PIN);		//turn on load
	}
	else
	{
		PWR_LOAD_PORT &= ~(_BV(PWR_LOAD_PIN));	//turn off load
	}
}

void Power::run()
{
	static uint8_t ptr = 0, cnt=0;
	if(powerTimer.isTimeUp())	//every 10ms
	{
		if(adcStable < 10)	//count adcStable till 10
		{
			adcStable++;
		}
		volBuffer[ptr] = ADC;	//read ADC
		ptr = (ptr+1) % 5;		//ringbuffer
		cnt++;
		if(cnt > 100)
		{
			cnt=0;
			if(gracePeriod > 0)
			{
				gracePeriod--;
			}
		}
	}
}
