/*
 * data.h
 *
 *  Created on: 30.01.2019
 *      Author: pmale
 */

#ifndef DATA_H_
#define DATA_H_

#include "PLATFORM.h"

class Data
{
public:
	typedef enum{
		DATA_INTERVAL = DIGIT_INTERVAL,
		DATA_DURATION = DIGIT_DURATION,
		DATA_SETUP_TEMP,
		DATA_CURRENT_TEMP,
		DATA_SIZE
	}data_type_t;

	static void Init();
	static void Set(data_type_t data_type, uint16_t val);
	static uint16_t Get(data_type_t data_type);
	static void Save();
	static void Increment(data_type_t data_type);
	static void Decrement(data_type_t data_type);
	static void decCountdown(uint8_t sec);
	static uint16_t getCountdownDisplay();
	static uint32_t getCountdown();
	static void resetCountdown();

private:
	enum data_adr{
		ADR_INIT_CONST = 0x00,
		ADR_INTERVAL = 0x04,
		ADR_DURATION = 0x08,
		ADR_TIME_COUNTER = 0x0C,
		ADR_SETUP_TEMP = 0x10,
		ADR_CURRENT_TEMP = 0x14,
	};
	#define DATA_INIT_CONST 0xF00DBABE
	#define DATA_INTERVAL_DEFAULT  120		//1..990 = 0.1..99.0
	#define DATA_DURATION_DEFAULT  30		//1..990 = 0.1..99.0
	#define DATA_SETUP_TEMP_DEFAULT 20		//20*0.5 deg C = 10 deg C

	static uint16_t data[DATA_SIZE];
	static uint32_t countdown;
};


#endif /* DATA_H_ */
