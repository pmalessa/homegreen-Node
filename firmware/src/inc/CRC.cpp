#include "CRC.hpp"

uint16_t CRC::currentCRC = 0xFFFF;

void CRC::AddByte(uint8_t value) 
{
	uint8_t i;
	for (i = 0; i < 8; i++) {
		if (((currentCRC & 0x8000) >> 8) ^ (value & 0x80)){
			currentCRC = (currentCRC << 1)  ^ CRC16_CCITT;
		}else{
			currentCRC = (currentCRC << 1);
		}
		value <<= 1;
	}
}

void CRC::AddWord(uint16_t value) 
{
    CRC::AddByte(LOW_BYTE(value));
    CRC::AddByte(HIGH_BYTE(value));
}

void CRC::AddDWord(uint32_t value) 
{
    CRC::AddWord(LOW_WORD(value));
    CRC::AddWord(HIGH_WORD(value));
}

void CRC::Reset()
{
    currentCRC = 0xFFFF;
}

uint16_t CRC::getValue()
{
    return currentCRC;
}