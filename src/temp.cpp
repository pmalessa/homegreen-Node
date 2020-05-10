/*
 * TEMP.c
 *
 *  Created on: 13.02.2019
 *      Author: pmale
 */

#include "temp.hpp"

DeltaTimer Temp::tempTimer;

void Temp::Init()
{
	i2c_init();
	tempTimer.setTimeStep(1000);
}

void Temp::Sleep()
{
	I2C_SCL_DDR |= (1 << I2C_SCL_PIN); //I2C Pin as Output while Temp Sensor unused
	I2C_SDA_DDR |= (1 << I2C_SDA_PIN); 

	I2C_SCL_PORT &= ~(1 << I2C_SCL_PIN);	//Set Low
	I2C_SDA_PORT &= ~(1 << I2C_SDA_PIN);
}

void Temp::Wakeup()
{
	i2c_init();
}

float Temp::getFloat(uint16_t val)
{
	return (float)val / 256.0f;
}

//call periodically to update current temp
void Temp::run()
{
	static uint16_t result;
	if(tempTimer.isTimeUp())
	{
		if (read16bitRegister(LM75A_REGISTER_TEMP, &result) == true)	//if successful
		{
			Data::Set(Data::DATA_CURRENT_TEMP,(int16_t)(result >> 7)*5); //turn into 0.1 Degree resolution -> 22.5 Deg => 225
		}
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
