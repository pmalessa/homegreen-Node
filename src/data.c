/*
 * data.c
 *
 *  Created on: 30.01.2019
 *      Author: pmale
 */

#include "PLATFORM.h"
#include "data.h"
#include "avr/eeprom.h"

uint16_t data[DATA_SIZE];

uint32_t countdown = 0;

void data_init()
{
	if(!(eeprom_read_dword((uint32_t *)DATA_ADR_INIT_CONST) == DATA_INIT_CONST))
	{
		data_set(DATA_INTERVAL,DATA_INTERVAL_DEFAULT);	//set default values
		data_set(DATA_DURATION,DATA_DURATION_DEFAULT);
		data_save();
		eeprom_write_dword((uint32_t *)DATA_ADR_INIT_CONST, DATA_INIT_CONST);	//set init constant
	}
	data[DATA_INTERVAL] = eeprom_read_word((uint16_t *)DATA_ADR_INTERVAL);
	data[DATA_DURATION] = eeprom_read_word((uint16_t *)DATA_ADR_DURATION);
	data_resetCountdown();
}

void data_increment(data_type_t data_type)
{
	if(data[data_type] < 100)	//smaller 100, +1 steps
	{
		data[data_type] += 1;
	}
	else if(data[data_type] < 990)	//higher 100, +10 steps, smaller 990
	{
		data[data_type] += 10;
	}
}

void data_decrement(data_type_t data_type)
{
	if(data[data_type] > 1)
	{
		if(data[data_type] <= 100)	//smaller equal 100, -1 steps
		{
			data[data_type] -= 1;
		}
		else						//higher 100, -10 steps
		{
			data[data_type] -= 10;
		}
	}
}

void data_set(data_type_t data_type, uint16_t val)
{
	if(val > 0 && val <= 990)
	{
		data[data_type] = val;
	}
}

uint16_t data_get(data_type_t data_type)
{
	return data[data_type];
}

void data_decCountdown(uint8_t sec)
{
	if(countdown > sec)
	{
		countdown -= sec;
	}
	else
	{
		countdown = 0;
	}
}

void data_resetCountdown()	//Todo: Maybe here is the error...
{
	countdown = (uint32_t)data[DATA_INTERVAL]*360;	//converted to seconds
}

uint32_t data_getCountdown()
{
	return countdown;
}

uint16_t data_getCountdownDisplay()
{
	return (countdown/360);	//Todo: or here...
}

void data_save()
{
	eeprom_write_word((uint16_t *)DATA_ADR_INTERVAL, data[DATA_INTERVAL]);	//save interval
	eeprom_write_word((uint16_t *)DATA_ADR_DURATION, data[DATA_DURATION]);	//save duration
}
