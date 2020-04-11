/*
 * data.c
 *
 *  Created on: 30.01.2019
 *      Author: pmale
 */

#include "data.hpp"
#include "avr/eeprom.h"

uint16_t Data::data[DATA_SIZE] = {0};
uint32_t Data::countdown = 0;

void Data::Init()
{
	if(!(eeprom_read_dword((uint32_t *)ADR_INIT_CONST) == DATA_INIT_CONST))
	{
		setDefault();
	}
	data[DATA_INTERVAL] = eeprom_read_word((uint16_t *)ADR_INTERVAL);
	data[DATA_DURATION1] = eeprom_read_word((uint16_t *)ADR_DURATION1);
	data[DATA_DURATION2] = eeprom_read_word((uint16_t *)ADR_DURATION2);
	data[DATA_DURATION3] = eeprom_read_word((uint16_t *)ADR_DURATION3);
	resetCountdown();
}

void Data::Increment(data_type_t data_type)
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

void Data::Decrement(data_type_t data_type)
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

void Data::Set(data_type_t data_type, uint16_t val)
{
	switch (data_type) {
		case DATA_INTERVAL:
		case DATA_DURATION1:
		case DATA_DURATION2:
		case DATA_DURATION3:
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

uint16_t Data::Get(data_type_t data_type)
{
	switch (data_type) {
		case DATA_INTERVAL:
		case DATA_DURATION1:
		case DATA_DURATION2:
		case DATA_DURATION3:
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

void Data::decCountdown(uint8_t sec)
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

void Data::resetCountdown()
{
	countdown = (uint32_t)data[DATA_INTERVAL]*360;	//converted to seconds
}

uint32_t Data::getCountdown()
{
	return countdown;
}

uint16_t Data::getCountdownDisplay()
{
	return (countdown/360);
}

void Data::Save()
{
	cli();
	_delay_ms(10);
	eeprom_write_word((uint16_t *)ADR_INTERVAL, data[DATA_INTERVAL]);		//save interval
	eeprom_write_word((uint16_t *)ADR_DURATION1, data[DATA_DURATION1]);		//save duration
	eeprom_write_word((uint16_t *)ADR_DURATION2, data[DATA_DURATION2]);		//save duration
	eeprom_write_word((uint16_t *)ADR_DURATION3, data[DATA_DURATION3]);		//save duration
	eeprom_write_word((uint16_t *)ADR_SETUP_TEMP, data[DATA_SETUP_TEMP]);	//save setup temp
	_delay_ms(10);
	sei();
}

void Data::setDefault()
{
	Set(DATA_INTERVAL,DATA_INTERVAL_DEFAULT);	//set default values
	Set(DATA_DURATION1,DATA_DURATION1_DEFAULT);
	Set(DATA_DURATION2,DATA_DURATION2_DEFAULT);
	Set(DATA_DURATION3,DATA_DURATION3_DEFAULT);
	Set(DATA_SETUP_TEMP,DATA_SETUP_TEMP_DEFAULT);
	Save();
	eeprom_write_dword((uint32_t *)ADR_INIT_CONST, DATA_INIT_CONST);	//set init constant
}