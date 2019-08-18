/*
 * buzzer.c
 *
 *  Created on: 28.01.2019
 *      Author: pmale
 */

#include "PLATFORM.h"
#include "buzzer.h"

typedef struct{
	uint16_t freq;
	uint16_t len;
}tone_t;

//circular buffer
tone_t* buffer[10];	//an array of tone_t pointers
uint8_t head = 0;
uint8_t tail = 0;

void set_freq(uint16_t hz);

uint16_t freqTable[41] ={
	0,		//not used
	624,	//100Hz
	311,	//200Hz
	208,	//300Hz
	156,	//400Hz
	125,	//500Hz
	103,	//600Hz
	89,		//700Hz
	78,		//800Hz
	69,		//900Hz
	62,		//1000Hz
	56,		//1100Hz
	52,		//1200Hz
	48,		//1300Hz
	44,		//1400Hz
	41,		//1500Hz
	39,		//1600Hz
	36,		//1700Hz
	34,		//1800Hz
	32,		//1900Hz
	31,		//2000Hz
	29,		//2100Hz
	28,		//2200Hz
	27,		//2300Hz
	26,		//2400Hz
	25,		//2500Hz
	24,		//2600Hz
	23,		//2700Hz
	22,		//2800Hz
	21,		//2900Hz
	20,		//3000Hz
	19,		//3100
	18,		//3200
	17,		//3300
	16,		//3400
	15,		//3500
	14,		//3600
	13,		//3700
	12,		//3800
	11,		//3900
	10		//4000
};

//freq_hz, length_ms
tone_t boot_pb[4] ={
	{20, 200},
	{25, 200},
	{30, 200},
	{0,0}};
tone_t boot_bat[4] ={
	{20, 200},
	{0, 200},
	{20, 200},
	{0,0}};
tone_t click[2] ={
	{30, 2},
	{0,0}};
tone_t heartbeat[2] ={
	{30, 5},
	{0,0}};
tone_t powUp[3] ={
	{20, 100},
	{30, 100},
	{0,0}};
tone_t powDown[3] ={
	{30, 100},
	{20, 100},
	{0,0}};
tone_t tone_alarm[5] ={
	{0,10},
	{35, 20},
	{0,15},
	{35, 20},
	{0,0}};

void buzzer_init()
{
	TCCR1A = (1<<COM1A0); 				//Enable A Output on Timer 1, toggle on Compare match
	TCCR1B = (1<< WGM12) | (1 << CS11);	//CTC, Prescaler 8
	TCCR1C = 0x00;
	
	BUZZER_DDR |= _BV(BUZZER_PIN); //Set Buzzer pin as output
	TCCR1A &= ~(1<<COM1A0);	//disable output
}

void buzzer_playTone(uint8_t tone_id)
{
	switch (tone_id) {
		case TONE_BOOT:
			buffer[head] = boot_pb;
			break;
		case TONE_BOOT2:
			buffer[head] = boot_bat;
			break;
		case TONE_POW_UP:
			buffer[head] = powUp;
			break;
		case TONE_POW_DOWN:
			buffer[head] = powDown;
			break;
		case TONE_HEARTBEAT:
			buffer[head] = heartbeat;
			break;
		case TONE_CLICK:
			buffer[head] = click;
			break;
		case TONE_ALARM:
			buffer[head] = tone_alarm;
			break;
		default:
			return;
			break;
	}
	head = (head + 1) %10; //advance pointer
}

uint8_t buzzer_isPlaying()
{
	return (head != tail);
}

//frequency = (F_CPU/(2*Pre)) * (1/(1+OCR1A))
//for prescaler 8:
//OCR1A = 0..65535
//freq = 1..62500 Hz
void set_freq(uint16_t hz)
{
	if(hz == 0)
	{
		TCCR1A &= ~(1<<COM1A0);	//disable output
	}
	else if(hz <=40)
	{
		TCCR1A |= (1<<COM1A0);	//enable output
		OCR1A = freqTable[hz];	//62500/hz -1
	}
}

void buzzer_SyncTask()	//1ms
{
	static uint16_t cnt = 0;	//counter for tone length
	static uint8_t ptr = 0;		//pointer of tone part
	if(head != tail)		//if tone available
	{
		if(buffer[tail][ptr].freq == 0 && buffer[tail][ptr].len == 0) //end of tone
		{
			cnt = 0;
			ptr = 0;
			set_freq(0); //turn off
			tail = (tail + 1) % 10; //advance tail
		}
		else if(cnt == 0)	//start of tone
		{
			set_freq(buffer[tail][ptr].freq);
			cnt++;
		}
		else if(cnt >= buffer[tail][ptr].len)	//end of tone part, next tone part
		{
			cnt = 0;
			ptr++;	//move to next buffer position
		}
		else
		{
			cnt++;
		}
	}
}
