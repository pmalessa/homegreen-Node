/*
 * PUMP.c
 *
 *  Created on: 17.01.2019
 *      Author: pmale
 */

#include "PLATFORM.h"
#include "pump.h"

volatile uint16_t pump_running = 0;

void pump_init()
{
	PUMP_DDR |= _BV(PUMP_PIN); //Set Pump Pin as Output
	pump_running = 0;
}

void pump_vSyncTask()	//1000ms
{
	if(pump_running > 0)	//if enabled
	{
		PUMP_PORT |= _BV(PUMP_PIN);	//turn on
		pump_running--;	//countdown
	}
	else
	{
		PUMP_PORT &= ~(_BV(PUMP_PIN));	//turn off
	}
}

void pump_enable(uint16_t val)	//0.1min steps
{
	pump_running = val*6;
}

void pump_disable()
{
	pump_running = 0;
}

uint16_t pump_getCountdown()
{
	return pump_running;
}
