/*
 * TinyWire.h
 *
 *  Created on: 13.02.2019
 *      Author: pmale
 */

#ifndef TINYWIRE_H_
#define TINYWIRE_H_

#define BUFFER_LENGTH 32

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

void TinyWire_begin();
void TinyWire_end();
void TinyWire_beginTransmission(uint8_t address);
uint8_t TinyWire_endTransmission(uint8_t sendStop);
uint8_t TinyWire_writeOne(uint8_t data);
uint8_t TinyWire_write(const uint8_t *data, uint8_t quantity);
uint8_t TinyWire_requestFrom(uint8_t address,uint8_t quantity,uint32_t iaddress,uint8_t isize,uint8_t sendStop);
int TinyWire_available();
int TinyWire_read();
int TinyWire_peek();


#endif /* TINYWIRE_H_ */
