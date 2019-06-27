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


volatile uint8_t playing = 0;

void set_freq(uint16_t hz);

//freq_hz, length_ms
tone_t boot_pb[4] ={
	{1000, 100},
	{1500, 100},
	{2000, 100},
	{0,0}};
tone_t boot_bat[4] ={
	{2000, 200},
	{0, 200},
	{2000, 200},
	{0,0}};
tone_t click[2] ={
	{3000, 2},
	{0,0}};
tone_t heartbeat[2] ={
	{4000, 5},
	{0,0}};
tone_t powUp[5] ={
	{2000, 50},
	{3000, 50},
	{0,0}};
tone_t powDown[5] ={
	{3000, 50},
	{2000, 50},
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
	else
	{
		TCCR1A |= (1<<COM1A0);	//enable output
		OCR1A = (62500/hz)-1;
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
			return;
		}
		else if(cnt == 0)	//start of tone
		{
			set_freq(buffer[tail][ptr].freq);
		}
		else if(cnt >= buffer[tail][ptr].len)	//end of tone part, next tone part
		{
			cnt = 0;
			ptr++;	//move to next buffer position
			return;
		}
		cnt++;
	}
}
