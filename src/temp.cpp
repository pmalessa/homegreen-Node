/*
 * TEMP.c
 *
 *  Created on: 13.02.2019
 *      Author: pmale
 */

#include "temp.hpp"

DeltaTimer Temp::tempTimer;
uint8_t Temp::initialized = false;

void Temp::Init()
{
	tempTimer.setTimeStep(1000);
	Wakeup();
}

void Temp::Sleep()
{
	initialized = false;
	PRR |= (1 << PRTWI);	//Disable TWI
	I2C_SCL_DDR |= (1 << I2C_SCL_PIN); //I2C Pin as Output while Temp Sensor unused
	I2C_SDA_DDR |= (1 << I2C_SDA_PIN); 

	I2C_SCL_PORT &= ~(1 << I2C_SCL_PIN);	//Set Low
	I2C_SDA_PORT &= ~(1 << I2C_SDA_PIN);
}

void Temp::Wakeup()
{
	PRR &= ~(1 << PRTWI);	//Enable TWI
	i2c_init();
	initialized = true;
}

//call periodically to update current temp
void Temp::run()
{
	static uint16_t result;
	static int16_t signedResult;
	if(initialized)
	{
		if(tempTimer.isTimeUp())
		{
			if (read16bitRegister(LM75A_REGISTER_TEMP, &result) == true)	//if successful
			{
				signedResult = (int16_t)result;		//D10 D9 D8 D7 D6  D5 D4 D3 | D2 D1 D0  X  X  X  X  X
				signedResult = signedResult >> 5;	//  X  X  X  X  X D10 D9 D8 | D7 D6 D5 D4 D3 D2 D1 D0	//0.125Deg per 1 Bit
				signedResult = (signedResult*5)>>2;	//convert to 0.1 Deg per bit -> 8bit/deg to 10bit/deg = x 5/4
				Data::SetTemp(Data::DATA_CURRENT_TEMP,signedResult); //turn into 0.1 Degree resolution -> 22.5 Deg => 225
			}
		}
	}
}

//Adjust temperature by TEMP_ADJ_FACTOR in percent per degree of temperature
//only adjust if TEMP_MIN is reached.
//value returned is a percentage to multiply with duration to get the value to add
uint16_t Temp::getTempAdjustFactor()
{
	return 0;
}
