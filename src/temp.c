/*
 * TEMP.c
 *
 *  Created on: 13.02.2019
 *      Author: pmale
 */

#include "PLATFORM.h"
#include "temp.h"
#include "TinyWire.h"

bool read16bitRegister(uint8_t reg, uint16_t* response);

void temp_init()
{
	TinyWire_begin();
}

float temp_getTemperature()
{
	uint16_t result;
	if (!read16bitRegister(LM75A_REGISTER_TEMP, &result))
	{
		return LM75A_INVALID_TEMPERATURE;
	}
	return (float)result / 256.0f;
}

bool read16bitRegister(uint8_t reg, uint16_t* response)
{
	uint8_t result;

	TinyWire_beginTransmission(0x48);
	TinyWire_writeOne(reg);
	result = TinyWire_endTransmission(true);
	// result is 0-4
	if (result != 0)
	{
		return false;
	}

	result = TinyWire_requestFrom(0x48, (uint8_t)2, 0, 0, true);
	if (result != 2)
	{
		return false;
	}
	uint8_t part1 = TinyWire_read();
	uint8_t part2 = TinyWire_read();

	//response = (Wire.read() << 8) | Wire.read();
	uint16_t temp = part1 << 8 | part2;
	*response = part1 << 8 | part2;
	return true;
}
