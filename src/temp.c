/*
 * TEMP.c
 *
 *  Created on: 13.02.2019
 *      Author: pmale
 */

#include "PLATFORM.h"
#include "temp.h"
#include "i2c_master.h"

bool read16bitRegister(uint8_t reg, uint16_t* response);

void temp_init()
{
	i2c_master_init();
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
	uint8_t msg[2];

	msg[0] = (LM75A_DEFAULT_ADDRESS<<TWI_ADR_BITS) | (false<<TWI_READ_BIT);	//set write bit
	msg[1] = reg;
	i2c_master_startData(msg,2);

	while(i2c_master_isBusy()); //wait

	msg[0] = (LM75A_DEFAULT_ADDRESS<<TWI_ADR_BITS) | (true<<TWI_READ_BIT);	//set read bit
	i2c_master_startData(msg,1);

	i2c_master_getData(msg,2);

	*response = msg[0] << 8 | msg[1];
	return true;
}
