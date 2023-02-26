/*
 * data.c
 *
 *  Created on: 30.01.2019
 *      Author: pmale
 */

#include "data.hpp"
#include "avr/eeprom.h"
#include "inc/CRC.hpp"
#include "Led.hpp"

uint16_t Data::data[DATA_SIZE] = {0};
uint32_t Data::countdown = 0;
uint8_t Data::ignoreStatus = 0, Data::savePending = false;
Data::statusAndStrengthUnion Data::statusAndStrength = {0};

void Data::Init()
{
	//if EEPROM not initialized or wrong Data Version
	if((eeprom_read_dword((uint32_t *)ADR_INIT_CONST) != DATA_INIT_CONST) || 
		(eeprom_read_word((uint16_t *)ADR_EEP_VERSION) != DATA_EEP_VERSION))
	{
		eeprom_write_word((uint16_t *)ADR_TOTAL_RUNTIME,0);
		setDefault();
		Led::Blink(LED_RED,3,100);
	}
	//load data
	data[DATA_INTERVAL] = eeprom_read_word((uint16_t *)ADR_INTERVAL);
	data[DATA_DURATION1] = eeprom_read_word((uint16_t *)ADR_DURATION1);
	data[DATA_DURATION2] = eeprom_read_word((uint16_t *)ADR_DURATION2);
	data[DATA_DURATION3] = eeprom_read_word((uint16_t *)ADR_DURATION3);
	data[DATA_TOTAL_RUNTIME] = eeprom_read_word((uint16_t *)ADR_TOTAL_RUNTIME);
	statusAndStrength.raw = eeprom_read_word((uint16_t *)ADR_STATUS);
	ignoreStatus = eeprom_read_word((uint16_t *)ADR_IGNORE_STATUS);

	//IF CRC not correct, reset Data
	if(eeprom_read_word((uint16_t *)ADR_CRC) != CalcCRC())
	{
		setDefault();
		Led::Blink(LED_REDGREEN,3,100);
	}
	
	resetCountdown();
}

//get CRC calculated from local data
uint16_t Data::CalcCRC()
{
	CRC::Reset();
	CRC::AddWord(eeprom_read_word((uint16_t *)ADR_INTERVAL));
	CRC::AddWord(eeprom_read_word((uint16_t *)ADR_DURATION1));
	CRC::AddWord(eeprom_read_word((uint16_t *)ADR_DURATION2));
	CRC::AddWord(eeprom_read_word((uint16_t *)ADR_DURATION3));
	CRC::AddWord(eeprom_read_word((uint16_t *)ADR_TOTAL_RUNTIME));
	CRC::AddWord(eeprom_read_word((uint16_t *)ADR_STATUS));
	CRC::AddWord(eeprom_read_word((uint16_t *)ADR_IGNORE_STATUS));
	return CRC::getValue();
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
	if(data_type == DATA_TOTAL_RUNTIME)
	{
		data[data_type] = val;
		return;
	}
	if(val > 0 && val <= 990)
	{
		data[data_type] = val;
	}
}

uint16_t Data::Get(data_type_t data_type)
{
	return data[data_type];
}

void Data::SetError(statusBit_t bit)
{
	if(ignoreStatus & _BV(bit))	//if Error is ignored, dont set it
	{
		return;
	}
	statusAndStrength.status |= _BV(bit);
	savePending = true;
}

uint8_t Data::GetErrors()
{
	return statusAndStrength.status;
}

void Data::ClearError(statusBit_t bit)
{
	statusAndStrength.status &= ~_BV(bit);
	savePending = true;
}

void Data::SetIgnoreError(statusBit_t bit)
{
	ignoreStatus |= _BV(bit);
	savePending = true;
}

void Data::SetPumpStrength(uint8_t id, uint8_t strength)
{
	statusAndStrength.strength = (strength << (id << 1));
	savePending = true;
}

uint8_t Data::GetPumpStrength(uint8_t id)
{
	return (statusAndStrength.strength >> (id << 1))&0x03;
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

void Data::setCustomCountdown(uint32_t sec)
{
	countdown = sec;
}

uint32_t Data::getCountdown()
{
	return countdown;
}

uint16_t Data::getCountdownDisplay()
{
	return (countdown/360);
}

void Data::setSavePending()
{
	savePending = true;
}

void Data::SaveConfig()
{
	if(savePending)
	{
		cli();
		eeprom_update_dword((uint32_t *)ADR_INIT_CONST, DATA_INIT_CONST);	//set init constant
		eeprom_update_word((uint16_t *)ADR_INTERVAL, data[DATA_INTERVAL]);		//save interval
		eeprom_update_word((uint16_t *)ADR_DURATION1, data[DATA_DURATION1]);		//save duration
		eeprom_update_word((uint16_t *)ADR_DURATION2, data[DATA_DURATION2]);		//save duration
		eeprom_update_word((uint16_t *)ADR_DURATION3, data[DATA_DURATION3]);		//save duration
		eeprom_update_word((uint16_t *)ADR_TOTAL_RUNTIME, data[DATA_TOTAL_RUNTIME]);		//save total runtime
		eeprom_update_word((uint16_t *)ADR_STATUS, statusAndStrength.raw);	//save status
		eeprom_update_word((uint16_t *)ADR_IGNORE_STATUS, ignoreStatus);	//save ignoreStatus
		eeprom_update_word((uint16_t *)ADR_EEP_VERSION, DATA_EEP_VERSION);	//save EEPROM Version
		eeprom_update_word((uint16_t *)ADR_CRC, CalcCRC());	//save CRC
		savePending = false;
		sei();
		Led::Blink(LED_REDGREEN,1,100);
	}
}

void Data::SaveError()
{
	cli();
	eeprom_update_word((uint16_t *)ADR_STATUS, statusAndStrength.raw);	//save status
	eeprom_update_word((uint16_t *)ADR_CRC, CalcCRC());	//save CRC
	Led::Blink(LED_REDGREEN,1,50);
	sei();
}

void Data::setDefault()
{
	Set(DATA_INTERVAL,DATA_INTERVAL_DEFAULT);	//set default values
	Set(DATA_DURATION1,DATA_DURATION1_DEFAULT);
	Set(DATA_DURATION2,DATA_DURATION2_DEFAULT);
	Set(DATA_DURATION3,DATA_DURATION3_DEFAULT);
	statusAndStrength.raw = 0;
	ignoreStatus = 0;
	savePending = true;
}

void Data::resetFromEEPROM()
{
	
	data[DATA_INTERVAL] = eeprom_read_word((uint16_t *)ADR_INTERVAL);
	data[DATA_DURATION1] = eeprom_read_word((uint16_t *)ADR_DURATION1);
	data[DATA_DURATION2] = eeprom_read_word((uint16_t *)ADR_DURATION2);
	data[DATA_DURATION3] = eeprom_read_word((uint16_t *)ADR_DURATION3);
}