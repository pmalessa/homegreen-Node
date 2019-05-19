/*
 * TEMP.c
 *
 *  Created on: 13.02.2019
 *      Author: pmale
 */

#include "PLATFORM.h"
#include "temp.h"
#include "i2cmaster.h"
#include "data.h"

bool read16bitRegister(uint8_t reg, uint16_t* response);

void temp_init()
{
	i2c_init();
}

float temp_getFloat(uint16_t val)
{
	return (float)val / 256.0f;
}

//call periodically to update current temp
void temp_updateTemp()
{
	uint16_t result;
	if (read16bitRegister(LM75A_REGISTER_TEMP, &result) == true)	//if successful
	{
		data_set(DATA_CURRENT_TEMP,(result >> 7)*5); //turn into 0.1 Degree resolution -> 22.5 Deg => 225
	}
}

bool read16bitRegister(uint8_t reg, uint16_t* response)
{
	if(i2c_start(LM75A_DEFAULT_ADDRESS+I2C_WRITE) == 1)
	{
		return false; //error
	}
	i2c_write(reg);
	i2c_rep_start(LM75A_DEFAULT_ADDRESS+I2C_READ);
	*response = i2c_readAck() << 8;
	*response |= i2c_readNak();
	i2c_stop();
	return true;
}

//Adjust temperature by TEMP_ADJ_FACTOR in percent per degree of temperature
//only adjust if TEMP_MIN is reached.
//value returned is a percentage to multiply with duration to get the value to add
uint16_t temp_getTempAdjustFactor()
{
	if(temp_getFloat(data_get(DATA_CURRENT_TEMP)) > TEMP_MIN)
	{
		float diff = temp_getFloat(data_get(DATA_CURRENT_TEMP) - data_get(DATA_SETUP_TEMP));
		if(diff < 0)	//current temp smaller than setup temp
		{
			return 0;	//no adjustment
		}
		else
		{
			return (uint16_t)diff*TEMP_ADJ_FACTOR;
		}
	}
	else
	{
		return 0;	//no adjustment
	}
}
