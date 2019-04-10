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
	if(!(eeprom_read_dword((uint32_t *)ADR_INIT_CONST) == DATA_INIT_CONST))
	{
		data_set(DATA_INTERVAL,DATA_INTERVAL_DEFAULT);	//set default values
		data_set(DATA_DURATION,DATA_DURATION_DEFAULT);
		data_set(DATA_SETUP_TEMP,DATA_SETUP_TEMP_DEFAULT);
		data_save();
		eeprom_write_dword((uint32_t *)ADR_INIT_CONST, DATA_INIT_CONST);	//set init constant
	}
	data[DATA_INTERVAL] = eeprom_read_word((uint16_t *)ADR_INTERVAL);
	data[DATA_DURATION] = eeprom_read_word((uint16_t *)ADR_DURATION);
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
	switch (data_type) {
		case DATA_INTERVAL:
		case DATA_DURATION:
		case DATA_PUMP_DURATION:
			if(val > 0 && val <= 990)
			{
				data[data_type] = val;
			}
			break;
		case DATA_SETUP_TEMP:
		case DATA_CURRENT_TEMP:
			data[data_type] = val;
			break;
		default:
			break;
	}
}

uint16_t data_get(data_type_t data_type)
{
	switch (data_type) {
		case DATA_INTERVAL:
		case DATA_DURATION:
		case DATA_PUMP_DURATION:
			return data[data_type];
			break;
		case DATA_SETUP_TEMP:
		case DATA_CURRENT_TEMP:
			return data[data_type];
			break;
		default:
			return 0;
			break;
	}
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

void data_resetCountdown()
{
	countdown = (uint32_t)data[DATA_INTERVAL]*360;	//converted to seconds
}

uint32_t data_getCountdown()
{
	return countdown;
}

uint16_t data_getCountdownDisplay()
{
	return (countdown/360);
}

void data_save()
{
	eeprom_write_word((uint16_t *)ADR_INTERVAL, data[DATA_INTERVAL]);		//save interval
	eeprom_write_word((uint16_t *)ADR_DURATION, data[DATA_DURATION]);		//save duration
	eeprom_write_word((uint16_t *)ADR_SETUP_TEMP, data[DATA_SETUP_TEMP]);	//save setup temp
}
