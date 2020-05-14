/*
 * TEMP.h
 *
 *  Created on: 13.02.2019
 *      Author: pmale
 */

#ifndef TEMP_H_
#define TEMP_H_

#include "PLATFORM.h"
#include "data.hpp"
#include "DeltaTimer.hpp"

extern "C"
{
#include "driver/i2cmaster.h"
}

class Temp
{
public:
    static void Init();
    static void Sleep();
    static void Wakeup();
    static void run();
    static float getFloat(int16_t val);
    static uint16_t getTempAdjustFactor();

private:
    static uint8_t initialized;
    #define LM75A_DEFAULT_ADDRESS		0x90		// 0x48 << 1
    #define LM75A_REGISTER_TEMP			0			// Temperature register (read-only)
    #define LM75A_REGISTER_CONFIG		1			// Configuration register

    #define LM75A_REGISTER_THYST		2			// Hysterisis register
    #define LM75A_REGISTER_TOS			3			// OS register
    #define LM75A_REGISTER_PRODID		7			// Product ID register - Only valid for Texas Instruments

    #define LM75_CONF_OS_COMP_INT		1			// OS operation mode selection
    #define LM75_CONF_OS_POL			2			// OS polarity selection
    #define LM75_CONF_OS_F_QUE			3			// OS fault queue programming

    #define LM75A_INVALID_TEMPERATURE	-1000.0f	// Just an arbritary value outside of the sensor limits

    #define TEMP_ADJ_FACTOR 10	//% per degree of temperature
    #define TEMP_MIN 15.0		//min temperature for temp adjustment

    static DeltaTimer tempTimer;

    static bool read16bitRegister(uint8_t reg, uint16_t* response)
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
};

#endif /* TEMP_H_ */
