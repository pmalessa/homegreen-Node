/*
 * power.c
 *
 *  Created on: 26.02.2019
 *      Author: pmale
 */

#include "PLATFORM.h"
#include "power.h"

uint16_t volatile curVol = 3000;
uint8_t volatile powerstate = 0;
void power_dummy_callback()
{
}
void (*power_callback)(void) = &power_dummy_callback;

uint16_t adcReadChannel(uint8_t channel);
uint16_t measureVoltage();

void power_init() {
	//adc init
    ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0);	//DIV8
    ADCSRB = 0;
    measureVoltage();	//scrap measurement
    powerstate = power_isPowerConnected();
}

void power_setCallback(void (*func)(void))	//set pin change callback function
{
	power_callback = func;
}

uint16_t adcReadChannel(uint8_t channel) {
    ADMUX = channel;
    ADMUX |= (1<<REFS0);	//using VCC Reference
    ADCSRA |= _BV(ADSC);	//start conversion
    while(ADCSRA & (1 << ADSC));	//wait for ADC to complete
    uint16_t ret = ADC;
    return ret;
}

uint16_t measureVoltage(){
	uint16_t val = adcReadChannel(CHANNEL_1V1);
	uint32_t result = 1125300L / val; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
	return result;
}

//return if last measured CurVol lower than Threshold
uint8_t power_isPowerConnected()
{
	curVol = measureVoltage();
	if(curVol > POWER_THRESHOLD)
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
		PWR_IN_PORT |= _BV(PWR_IN_PIN);
	}
	else
	{
		PWR_IN_PORT &= ~(_BV(PWR_IN_PIN));
	}
}

void power_SyncTask()	//every second
{
	if(power_isPowerConnected() != powerstate)	//if power state changed
	{
		powerstate = power_isPowerConnected();
		power_callback();	//call callback
	}
}
