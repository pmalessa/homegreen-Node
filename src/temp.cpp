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

//call periodically to update current temp
void Temp::run()
{
	static uint16_t result;
	static int16_t signedResult;
	if(tempTimer.isTimeUp())
	{
		if (read16bitRegister(LM75A_REGISTER_TEMP, &result) == true)	//if successful
		{
			signedResult = (int16_t)result;		//D10 D9 D8 D7 D6  D5 D4 D3 | D2 D1 D0  X  X  X  X  X
			signedResult = signedResult >> 5;	//  X  X  X  X  X D10 D9 D8 | D7 D6 D5 D4 D3 D2 D1 D0	//0.125Deg per 1 Bit
			signedResult = (signedResult*5)/4;	//convert to 0.1 Deg per bit -> 8bit/deg to 10bit/deg = x 5/4
			Data::SetTemp(Data::DATA_CURRENT_TEMP,signedResult); //turn into 0.1 Degree resolution -> 22.5 Deg => 225
		}
	}
}

//Adjust temperature by TEMP_ADJ_FACTOR in percent per degree of temperature
//only adjust if TEMP_MIN is reached.
//value returned is a percentage to multiply with duration to get the value to add
uint16_t Temp::getTempAdjustFactor()
{
	if(getFloat(Data::GetTemp(Data::DATA_CURRENT_TEMP)) > TEMP_MIN)
	{
		float diff = getFloat(Data::GetTemp(Data::DATA_CURRENT_TEMP) - Data::GetTemp(Data::DATA_SETUP_TEMP));
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
