/*
 * display.c
 *
 *  Created on: 29.01.2019
 *      Author: pmale
 */

#include "PLATFORM.h"
#include "display.h"
#include "tm1637.h"

volatile uint8_t dotmask, brightness, dig[6], blinking,blinkingEnabled, init;
volatile uint16_t timeout = DISPLAY_TIMEOUT_S;
void setdot(uint8_t dot_pos, uint8_t val);

#define TM_DOT 0x80

uint8_t numToByte[] =
{
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
};

uint8_t bootAnimation[] =
{
		0x76, // H
		0x79, // E
		0x38, // L
		0x38, // L
		0x3F, // O
		0x00, // 0
		0x00, // 0
		0x00, // 0
		0x00, // 0
		0x00, // 0
};

bool display_boot()
{
	static uint8_t state = 0;
	if(state == 0)	//clear first
	{
		display_clear();
	}
	if(state > 11)	//done
	{
		state = 0;
		return true;
	}
	display_setByte(state%5, bootAnimation[state]);
	state++;
	return false;
}

void display_init()
{
	init = 0;
	tm1637_Init();
	dotmask = 0;
	blinking = 0;
	blinkingEnabled = 0;
	display_clear();
	init = 1;
	display_setbrightness(7);
}

void display_deInit()
{
	display_clear();
	display_update();
	init = 0;
	tm1637_deInit();
}

bool display_wakeAnimation()
{
	static uint8_t state = 0, toggle = 0;
	if(state == 0)	//clear first
	{
		display_clear();
		display_setbrightness(7);
	}
	if(state > 5)	//done
	{
		state = 0;
		return true;
	}
	switch (toggle) {
		case 0:
			display_setByte(state, 0x30);
			toggle = 1;
			break;
		default:
			display_setByte(state, 0x36);
			toggle = 0;
			state++;
			break;
	}
	display_setbrightness(7);
	return false;
}

void display_clear()
{
	for(uint8_t i=0;i<6;i++)
	{
		dig[i]=0;
	}
	dotmask = 0;
}

void display_resettimeout()
{
	timeout = DISPLAY_TIMEOUT_S;
}

uint8_t display_gettimeout()
{
	return timeout;
}

//left to right
void setdot(uint8_t dot_pos, uint8_t val)
{
	if(val)
	{
		dotmask |= _BV(dot_pos);
	}
	else
	{
		dotmask &= ~_BV(dot_pos);
	}
}

#define PUMPANIMATION_FRAMES 12
uint32_t pumpanimation[PUMPANIMATION_FRAMES] = {
		0x01010000,
		0x00010100,
		0x00000101,
		0x00000003,
		0x00000006,
		0x0000000C,
		0x00000808,
		0x00080800,
		0x08080000,
		0x18000000,
		0x30000000,
		0x21000000
};
void display_pumpanimation(uint16_t countdown)
{
	static uint8_t pos = 0;
	dotmask = 0;
	dig[0] = (pumpanimation[pos]&0xFF000000)>>24;
	dig[1] = (pumpanimation[pos]&0x00FF0000)>>16;
	dig[2] = (pumpanimation[pos]&0x0000FF00)>>8;
	dig[3] = (pumpanimation[pos]&0x000000FF)>>0;
	display_setValue(DIGIT_COUNTDOWN,countdown/6);
	pos++;
	if(pos == 12)
	{
		pos = 0;
	}
}


void display_setByte(uint8_t pos, uint8_t byte)
{
	if(byte & 0x80)
	{
		setdot(pos,1);
	}
	else
	{
		setdot(pos,0);
	}
	dig[pos]=byte;
}


//val = (0.1 .. 1.0,1.1 .. 10.0,11.0 .. 99.0) * 10 = 1..990
void display_setValue(digit_t digit, uint16_t val)
{
	uint8_t d = digit<<1;
	if(val > 990)	//set high limit
	{
		val = 990;
	}
	if(val > 99) //higher than 9.9
	{
		display_setByte(d,numToByte[val/100]);
		display_setByte(d+1,numToByte[(val%100)/10]);
		setdot(d, 0);
		setdot(d+1, 1);	//dot at second position
	}
	else
	{
		display_setByte(d,numToByte[val/10]);
		display_setByte(d+1,numToByte[val%10]);
		setdot(d, 1);	//dot at first position
		setdot(d+1, 0);
	}
}

//call till return==1
uint8_t display_fadeUp(uint8_t val)
{
	if(brightness < val)
	{
		display_setbrightness(brightness+1);
		return 0;
	}
	else
	{
		return 1;
	}
}

//call till return==1
uint8_t display_fadeDown(uint8_t val)
{
	if(brightness > val)
	{
		display_setbrightness(brightness-1);
		return 0;
	}
	else
	{
		return 1;
	}
}

//0..7
void display_setbrightness(uint8_t val)
{
	if(val < 8)
	{
		brightness = val;
		tm1637_setBrightness(val);
	}
}

void display_setblinking(digit_t digit)
{
	blinkingEnabled = digit+1;
	blinking = 0;
}

void display_clearBlinking()
{
	blinkingEnabled = 0;
}

void dsend(uint8_t pos, uint8_t byte)
{
	tm1637_setByte(pos,byte | (dotmask & (1 << pos) ? TM_DOT : 0));
}

void display_update()
{
	if(init == 1)
	{
		if(!blinkingEnabled)
		{
			for(uint8_t i=0;i<6;i++)
			{
				dsend(i,dig[i]);
			}
		}
		else
		{
			if(blinking < 6)	//on
			{
				for(uint8_t i=0;i<6;i++)
				{
					dsend(i,dig[i]);
				}
			}
			else
			{
				switch (blinkingEnabled) {
					case 1: //DIGIT_INTERVAL
						dsend(0,0);
						dsend(1,0);
						dsend(2,dig[2]);
						dsend(3,dig[3]);
						dsend(4,dig[4]);
						dsend(5,dig[5]);
						break;
					case 2: //DIGIT_DURATION
						dsend(0,dig[0]);
						dsend(1,dig[1]);
						dsend(2,0);
						dsend(3,0);
						dsend(4,dig[4]);
						dsend(5,dig[5]);
						break;
					case 3: //DIGIT_COUNTDOWN
						dsend(0,dig[0]);
						dsend(1,dig[1]);
						dsend(2,dig[2]);
						dsend(3,dig[3]);
						dsend(4,0);
						dsend(5,0);
						break;
					default:
						break;
				}
			}
		}
	}
}

void display_SyncTask() //100ms
{
	blinking++;
	if(blinking > 10)
	{
		blinking = 0;
		if(timeout > 0)	//every second
		{
			timeout--;
		}
	}
}
