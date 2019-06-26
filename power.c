/*
 * power.c
 *
 *  Created on: 26.02.2019
 *      Author: pmale
 */

#include "PLATFORM.h"
#include "power.h"

uint16_t volatile curVol = 3000;
uint8_t volatile curPowerState = 0;
void power_dummy_callback()
{
}
void (*power_callback)(void) = &power_dummy_callback;

uint16_t adcReadChannel(uint8_t channel);
uint16_t measureVoltage();

void power_init() {
    ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0);	//DIV8
    ADCSRB = 0;
    measureVoltage();	//scrap measurement
    curPowerState = power_isPowerConnected();
    PWR_IN1_DDR |= _BV(PWR_IN1_PIN);
    PWR_IN2_DDR |= _BV(PWR_IN2_PIN);
    PWR_LOAD_DDR |= _BV(PWR_LOAD_PIN);

    PWR_LOAD_PORT &= ~(_BV(PWR_LOAD_PIN));	//turn off load
    power_setInputPower(0);
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
		PWR_IN1_PORT |= _BV(PWR_IN1_PIN); 		//turn on PB1
		PWR_LOAD_PORT |= _BV(PWR_LOAD_PIN);		//turn on load
		_delay_ms(500);
		PWR_LOAD_PORT &= ~(_BV(PWR_LOAD_PIN));	//turn off load
	}
	else
	{
		PWR_IN1_PORT &= ~(_BV(PWR_IN1_PIN));
	}
}

void power_SyncTask()	//every second
{
	static uint8_t newPowerState = 0;
	newPowerState = power_isPowerConnected();	//measure PowerState
	if(newPowerState != curPowerState)			//if PowerState changed
	{
		curPowerState = newPowerState;			//save new PowerState
		power_callback();						//call callback to notify
	}
}
