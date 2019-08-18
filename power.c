/*
 * power.c
 *
 *  Created on: 26.02.2019
 *      Author: pmale
 */

#include "PLATFORM.h"
#include "power.h"

uint8_t LoadCounter = 0;

uint16_t adcReadChannel(uint8_t channel);
uint16_t measureVoltage();

uint16_t volBuffer[5];
volatile uint8_t adcStable = 0;

void power_init() {
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
	
	adcStable = 0;
}

void power_deInit()
{
	ADCSRA = 0;	//disable ADC
	//disable ADC in PRR
}

uint16_t measureVoltage(){
	uint32_t val = 0;
	for(uint8_t i=0;i<5;i++)
	{
		val +=volBuffer[i];
	}
	val = val / 5;
	uint32_t result = 1125300L / val; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
	return result;
}

uint8_t power_isAdcStable()
{
	return (adcStable == 10);	//ADC is stable once 10 measurements are buffered
}

uint8_t power_isPowerConnected()	//return if last measured CurVol lower than Threshold
{
	/*
	if(PWR_5V_PINREG & (1 << PWR_5V_PIN))
	{
		return 1;
	}
	else
	{
		return 0;
	}
	*/
	uint16_t curVol = measureVoltage();
	if(curVol > POWER_THRESHOLD)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t power_isPowerLow()	//return if last measured CurVol lower than Threshold
{
	uint16_t curVol = measureVoltage();
	if(curVol < LOWVOLTAGE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void power_setInputPower(uint8_t state)
{
	if (state == 1)
	{
		PWR_IN_PORT &= ~(_BV(PWR_IN_PIN)); 		//turn on PB
		PWR_LOAD_PORT |= _BV(PWR_LOAD_PIN);		//turn on load
		LoadCounter = 50;
	}
	else
	{
		PWR_IN_PORT |= (_BV(PWR_IN_PIN));	//turn off PB
		LoadCounter = 0;
	}
}

void power_SyncTask()	//every 10ms
{
	static uint8_t ptr = 0;
	if(adcStable < 10)	//count adcStable till 10
	{
		adcStable++;
	}
	if(LoadCounter)
	{
		LoadCounter--;
		PWR_LOAD_PORT |= _BV(PWR_LOAD_PIN);		//turn on load
	}
	else
	{
		PWR_LOAD_PORT &= ~(_BV(PWR_LOAD_PIN));	//turn off load
	}
	volBuffer[ptr] = ADC;	//read ADC
	ptr = (ptr+1) % 5;		//ringbuffer
}
