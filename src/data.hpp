/*
 * data.h
 *
 *  Created on: 30.01.2019
 *      Author: pmale
 */

#ifndef DATA_H_
#define DATA_H_

#include "PLATFORM.h"
#include "power.hpp"

class Data
{
public:
	typedef enum{
		DATA_INTERVAL = DIGIT_INTERVAL,
		DATA_DURATION1 = DIGIT_DURATION,
		DATA_DURATION2,
		DATA_DURATION3,
		DATA_TOTAL_RUNTIME,
		DATA_SIZE
	}data_type_t;

	typedef enum{
		DATA_SETUP_TEMP,
		DATA_CURRENT_TEMP,
		TEMPDATA_SIZE
	}temp_type_t;

	typedef enum{
		STATUS_PB_ERR = 1,
		STATUS_P1_ERR,
		STATUS_P2_ERR,
		STATUS_P3_ERR,
		STATUS_EP_ERR,
	}statusBit_t;

	static void Init();
	static uint16_t CalcCRC();
	static void Set(data_type_t data_type, uint16_t val);
	static uint16_t Get(data_type_t data_type);
	static void SetError(statusBit_t bit);
	static void ClearError(statusBit_t bit);
	static uint8_t GetErrors();
	static void SetIgnoreError(statusBit_t bit);
	static void SetTemp(temp_type_t temp_type, int16_t val);
	static int16_t GetTemp(temp_type_t temp_type);
	static void Save();
	static void Increment(data_type_t data_type);
	static void Decrement(data_type_t data_type);
	static void decCountdown(uint8_t sec);
	static uint16_t getCountdownDisplay();
	static uint32_t getCountdown();
	static void resetCountdown();
	static void setCustomCountdown(uint32_t sec);
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
		ADR_TOTAL_RUNTIME = 0x14,
		ADR_STATUS = 0x16,
		ADR_IGNORE_STATUS = 0x18,
		ADR_EEP_VERSION = 0x30,
		ADR_CRC = 0x32
	};
	#define DATA_INIT_CONST 0xF00DBABE
	#define DATA_INTERVAL_DEFAULT  120		//1..990 = 0.1..99.0
	#define DATA_DURATION1_DEFAULT  20		//1..990 = 0.1..99.0
	#define DATA_DURATION2_DEFAULT  30		//1..990 = 0.1..99.0
	#define DATA_DURATION3_DEFAULT  10		//1..990 = 0.1..99.0
	#define DATA_SETUP_TEMP_DEFAULT 200		//200*0.1 deg C = 20 deg C
	#define DATA_EEP_VERSION 10				//1.0

	static uint16_t data[DATA_SIZE];
	static int16_t tempdata[TEMPDATA_SIZE];
	static uint32_t countdown;
	static uint8_t status, ignoreStatus;
};


#endif /* DATA_H_ */
