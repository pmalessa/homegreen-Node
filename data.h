/*
 * data.h
 *
 *  Created on: 30.01.2019
 *      Author: pmale
 */

#ifndef DATA_H_
#define DATA_H_

#define DATA_INIT_CONST 0xF00DBABE

#define DATA_INTERVAL_DEFAULT  120		//1..990 = 0.1..99.0
#define DATA_DURATION_DEFAULT  30		//1..990 = 0.1..99.0
#define DATA_PUMP_DURATION_DEFAULT 20	//2.0 min
#define DATA_SETUP_TEMP_DEFAULT 20		//20*0.5 deg C = 10 deg C

enum data_adr{
	ADR_INIT_CONST = 0x00,
	ADR_INTERVAL = 0x04,
	ADR_DURATION = 0x08,
	ADR_TIME_COUNTER = 0x0C,
	ADR_SETUP_TEMP = 0x10,
	ADR_CURRENT_TEMP = 0x14,
};

typedef enum{
	DATA_INTERVAL = DIGIT_INTERVAL,
	DATA_DURATION = DIGIT_DURATION,
	DATA_PUMP_DURATION,
	DATA_SETUP_TEMP,
	DATA_CURRENT_TEMP,
	DATA_SIZE
}data_type_t;

void data_init();
void data_set(data_type_t data_type, uint16_t val);
uint16_t data_get(data_type_t data_type);
void data_save();
void data_increment(data_type_t data_type);
void data_decrement(data_type_t data_type);
void data_decCountdown(uint8_t sec);
uint16_t data_getCountdownDisplay();
uint32_t data_getCountdown();
void data_resetCountdown();


#endif /* DATA_H_ */
