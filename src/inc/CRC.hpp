/*
 * crc.h
 *
 *  Created on: 30.01.2019
 *      Author: pmale
 */

#ifndef CRC_H_
#define CRC_H_

#include "PLATFORM.h"

class CRC
{
    #define CRC16_CCITT	0x1021		// X.25, V.41, HDLC FCS, Bluetooth, ...
public:
	static void AddByte(uint8_t value);
    static void AddWord(uint16_t value);
    static void AddDWord(uint32_t value);
    static uint16_t getValue();
    static void Reset();

private:
	static uint16_t currentCRC;
};


#endif /* DATA_H_ */
