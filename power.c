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

void power_init() {
    ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0);	//DIV8
    ADCSRB = 0;
    measureVoltage();	//scrap measurement
    PWR_IN1_DDR |= _BV(PWR_IN1_PIN);
    PWR_IN2_DDR |= _BV(PWR_IN2_PIN);
    PWR_LOAD_DDR |= _BV(PWR_LOAD_PIN);

    PWR_LOAD_PORT &= ~(_BV(PWR_LOAD_PIN));	//turn off load
	PWR_IN2_PORT &= ~(_BV(PWR_IN2_PIN));	//turn off PB2
    power_setInputPower(0);
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

uint8_t power_isPowerConnected()	//return if last measured CurVol lower than Threshold
{
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

void power_setInputPower(uint8_t state)
{
	if (state == 1)
	{
		PWR_IN1_PORT |= _BV(PWR_IN1_PIN); 		//turn on PB
		PWR_LOAD_PORT |= _BV(PWR_LOAD_PIN);		//turn on load
		LoadCounter = 5;
	}
	else
	{
		PWR_IN1_PORT &= ~(_BV(PWR_IN1_PIN));	//turn off PB
		LoadCounter = 0;
	}
}

void power_SyncTask()	//every 100ms
{
	if(LoadCounter)
	{
		LoadCounter--;
		PWR_LOAD_PORT |= _BV(PWR_LOAD_PIN);		//turn on load
	}
	else
	{
		PWR_LOAD_PORT &= ~(_BV(PWR_LOAD_PIN));	//turn off load
	}
}