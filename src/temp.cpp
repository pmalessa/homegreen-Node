/*
 * TEMP.c
 *
 *  Created on: 13.02.2019
 *      Author: pmale
 */

#include "temp.hpp"

void Temp::Init()
{
	i2c_init();
}

float Temp::getFloat(uint16_t val)
{
	return (float)val / 256.0f;
}

//call periodically to update current temp
void Temp::updateTemp()
{
	uint16_t result;
	if (read16bitRegister(LM75A_REGISTER_TEMP, &result) == true)	//if successful
	{
		Data::Set(Data::DATA_CURRENT_TEMP,(result >> 7)*5); //turn into 0.1 Degree resolution -> 22.5 Deg => 225
	}
}

//Adjust temperature by TEMP_ADJ_FACTOR in percent per degree of temperature
//only adjust if TEMP_MIN is reached.
//value returned is a percentage to multiply with duration to get the value to add
uint16_t Temp::getTempAdjustFactor()
{
	if(getFloat(Data::Get(Data::DATA_CURRENT_TEMP)) > TEMP_MIN)
	{
		float diff = getFloat(Data::Get(Data::DATA_CURRENT_TEMP) - Data::Get(Data::DATA_SETUP_TEMP));
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
