/*
 * data.h
 *
 *  Created on: 30.01.2019
 *      Author: pmale
 */

#ifndef DATA_H_
#define DATA_H_

#define DATA_ADR_INIT_CONST 	0x11 		//4 byte
#define DATA_ADR_INTERVAL		0x20		//2 byte
#define DATA_ADR_DURATION		0x24		//2 byte
#define DATA_ADR_TIME_COUNTER 	0x30		//4 byte

#define DATA_INIT_CONST 0xF00DBABE

#define DATA_INTERVAL_DEFAULT  120		//1..990 = 0.1..99.0
#define DATA_DURATION_DEFAULT  30		//1..990 = 0.1..99.0
#define DATA_PUMP_DURATION_DEFAULT 20	//2.0 min

typedef enum{
	DATA_INTERVAL = DIGIT_INTERVAL,
	DATA_DURATION = DIGIT_DURATION,
	DATA_PUMP_DURATION,
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
