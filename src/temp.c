/*
 * TEMP.c
 *
 *  Created on: 13.02.2019
 *      Author: pmale
 */

#include "PLATFORM.h"
#include "temp.h"
#include "i2c_master.h"
#include "data.h"

bool read16bitRegister(uint8_t reg, uint16_t* response);

void temp_init()
{
	i2c_master_init();
}

float temp_getFloat(uint16_t val)
{
	return (float)val / 256.0f;
}

//call periodically to update current temp
temp_updateTemp()
{
	uint16_t result;
	if (read16bitRegister(LM75A_REGISTER_TEMP, &result) == true)	//if successful
	{
		data_set(DATA_CURRENT_TEMP,result);
	}
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
