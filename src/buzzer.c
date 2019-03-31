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

volatile uint8_t playing = 0;
volatile tone_t *buf;

tone_t boot[5] =
{
		//freq_hz, length_10ms
		{1000, 20},
		{1250, 20},
		{1500, 20},
		{2000, 20},
		{0,0}
};

void buzzer_init()
{
	TCCR1A = 0x00;	//clear registers
	TCCR1B = 0x00;
	TCCR1C = 0x00;

	TCCR1A = (1<<COM1A0); //Enable A Output on Timer 1, toggle on Compare match
	TCCR1B = (1<< WGM12) | (1 << CS11);	//CTC, Prescaler 8

	TOCPMSA0 = 0x00;	//Set Pin Mux Register 1 to select OC0A Output
	TOCPMSA1 = 0x00;
	TOCPMCOE = (1 << TOCC1OE);	//Enable Pin Mux 1

	DDRB |= _BV(PB2); //Set Buzzer pin as output
}

void buzzer_playTone(uint8_t tone_id)
{
	switch (tone_id) {
		case TONE_BOOT:
			playing = 1;
			buf = boot;
			break;
		default:
			break;
	}
}

uint8_t buzzer_isPlaying()
{
	return playing;
}

//frequency = (F_CPU/(2*Pre)) * (1/(1+OCR1A))
//for prescaler 8:
//OCR1A = 0..65535
//freq = 1..62500 Hz
void set_freq(uint16_t hz)
{
	OCR1A = (62500/hz)-1;
}

void buzzer_SyncTask()	//10ms
{
	static uint16_t cnt = 0;
	if(playing)
	{
		if((*buf).freq == 0 && (*buf).len == 0) //end of buffer
		{
			cnt = 0;
			playing = 0;
			return;
		}
		else if(cnt == 0)	//start of tone
		{
			set_freq((*buf).freq);
		}
		else if(cnt >= (*buf).len)	//end of tone
		{
			cnt = 0;
			buf++;	//move to next buffer position
			return;
		}
		cnt++;
	}
}
