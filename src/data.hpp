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
		DATA_DURATION1 = DIGIT_DURATION,
		DATA_DURATION2,
		DATA_DURATION3,
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
	static void setDefault();

private:
	enum data_adr{
		ADR_INIT_CONST = 0x00,
		ADR_INTERVAL = 0x04,
		ADR_DURATION1 = 0x06,
		ADR_DURATION2 = 0x08,
		ADR_DURATION3 = 0x0A,
		ADR_TIME_COUNTER = 0x0C,
		ADR_SETUP_TEMP = 0x10,
		ADR_CURRENT_TEMP = 0x12,
	};
	#define DATA_INIT_CONST 0xF00DBABE
	#define DATA_INTERVAL_DEFAULT  120		//1..990 = 0.1..99.0
	#define DATA_DURATION1_DEFAULT  20		//1..990 = 0.1..99.0
	#define DATA_DURATION2_DEFAULT  30		//1..990 = 0.1..99.0
	#define DATA_DURATION3_DEFAULT  10		//1..990 = 0.1..99.0
	#define DATA_SETUP_TEMP_DEFAULT 200		//200*0.1 deg C = 20 deg C

	static uint16_t data[DATA_SIZE];
	static uint32_t countdown;
};


#endif /* DATA_H_ */
