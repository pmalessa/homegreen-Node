/*
 * TEMP.h
 *
 *  Created on: 13.02.2019
 *      Author: pmale
 */

#ifndef TEMP_H_
#define TEMP_H_

#define LM75A_DEFAULT_ADDRESS		0x48		// Address is configured with pins A0-A2
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

void temp_init();
void temp_updateTemp();

#endif /* TEMP_H_ */
