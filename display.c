/*
 * display.c
 *
 *  Created on: 29.01.2019
 *      Author: pmale
 */

#include "PLATFORM.h"
#include "display.h"
#include "tm1637.h"

volatile uint8_t dotmask, brightness, dig[6], blinking;
volatile uint16_t timeout = DISPLAY_TIMEOUT_S;

void setdot(uint8_t dot_pos, uint8_t val);

void display_init()
{
	tm1637_Init();
	dotmask = 0;
	blinking = 0;
	tm1637_Clear();
	display_setbrightness(7);
	for(uint8_t i=0;i<6;i++)
	{
		dig[i]=0;
	}
}

void display_deInit()
{
	tm1637_deInit();
}

void display_boot()
{
	display_clear();
	display_setByte(0, 0x76); //H
	_delay_ms(100);
	display_setByte(1, 0x79); //E
	_delay_ms(100);
	display_setByte(2, 0x38); //L
	_delay_ms(100);
	display_setByte(3, 0x38); //L
	_delay_ms(100);
	display_setByte(4, 0x3F); //O
	_delay_ms(100);
	display_setByte(0, 0x00);
	_delay_ms(100);
	display_setByte(1, 0x00);
	_delay_ms(100);
	display_setByte(2, 0x00);
	_delay_ms(100);
	display_setByte(3, 0x00);
	_delay_ms(100);
	display_setByte(4, 0x00);
	_delay_ms(100);
}

void display_clear()
{
	tm1637_Clear();
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
	tm1637_setDots(dotmask);
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
	tm1637_setDots(0);
	tm1637_setByte(0,(pumpanimation[pos]&0xFF000000)>>24);
	tm1637_setByte(1,(pumpanimation[pos]&0x00FF0000)>>16);
	tm1637_setByte(2,(pumpanimation[pos]&0x0000FF00)>>8);
	tm1637_setByte(3,(pumpanimation[pos]&0x000000FF)>>0);
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
	tm1637_setByte(pos,byte);
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
		dig[d] = val/100;
		dig[d+1] = (val%100)/10;
		setdot(d, 0);
		setdot(d+1, 1);	//dot at second position
		tm1637_setDigit(d,dig[d]);
		tm1637_setDigit(d+1,dig[d+1]);
	}
	else
	{
		dig[d] = val/10;
		dig[d+1] = val%10;
		setdot(d, 1);	//dot at first position
		setdot(d+1, 0);
		tm1637_setDigit(d,dig[d]);
		tm1637_setDigit(d+1,dig[d+1]);
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
	switch (digit) {
		case DIGIT_INTERVAL:
			tm1637_setDigit(2,dig[2]); //reset values in non blinking digits
			tm1637_setDigit(3,dig[3]);
			tm1637_setDigit(4,dig[4]);
			tm1637_setDigit(5,dig[5]);
			blinking = 1;
			break;
		case DIGIT_DURATION:
			tm1637_setDigit(0,dig[0]); //reset values in non blinking digits
			tm1637_setDigit(1,dig[1]);
			tm1637_setDigit(4,dig[4]);
			tm1637_setDigit(5,dig[5]);
			blinking = 2;
			break;
		case DIGIT_COUNTDOWN:
			tm1637_setDigit(0,dig[0]); //reset values in non blinking digits
			tm1637_setDigit(1,dig[1]);
			tm1637_setDigit(2,dig[2]);
			tm1637_setDigit(3,dig[3]);
			blinking = 3;
			break;
		default:
			break;
	}
}

void display_clearBlinking()
{
	tm1637_setDigit(0,dig[0]);	//reset values in non blinking digits
	tm1637_setDigit(1,dig[1]);
	tm1637_setDigit(2,dig[2]);
	tm1637_setDigit(3,dig[3]);
	tm1637_setDigit(4,dig[4]);
	tm1637_setDigit(5,dig[5]);
	blinking = 0;
}

void display_SyncTask() //100ms
{
	static uint8_t cnt = 0, cnt2 = 0, toggle = 0;
	cnt++;
	if(cnt>2)
	{
		cnt = 0;
		toggle++;
		switch (blinking) {
			case 1: //interval
				if(toggle%3)
				{	//set
					tm1637_setDigit(0,dig[0]);
					tm1637_setDigit(1,dig[1]);
				}
				else
				{	//clear
					tm1637_setByte(0,0);
					tm1637_setByte(1,0);
				}
				break;
			case 2: //duration
				if(toggle%3)
				{	//set
					tm1637_setDigit(2,dig[2]);
					tm1637_setDigit(3,dig[3]);
				}
				else
				{	//clear
					tm1637_setByte(2,0);
					tm1637_setByte(3,0);
				}
				break;
			case 3: //power
				if(toggle%3)
				{	//set
					tm1637_setDigit(4,dig[4]);
					tm1637_setDigit(5,dig[5]);
				}
				else
				{	//clear
					tm1637_setByte(4,0);
					tm1637_setByte(5,0);
				}
				break;
			default:
				break;
		}
	}
	cnt2++;
	if(cnt2>10)	//every second
	{
		cnt2 = 0;
		if(timeout > 0)
		{
			timeout--;
		}
	}
}
