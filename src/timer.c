/*
 * timer.c
 *
 *  Created on: 25.01.2019
 *      Author: pmale
 */

#include "PLATFORM.h"
#include "timer.h"

#include "pump.h"
#include "display.h"
#include "button.h"
#include "power.h"
#include "buzzer.h"
#include "temp.h"

void timer_init()
{
	//Init 1ms Timer, use Timer 0
	TCCR0A = 0x00;							//Register zuruecksetzen
	TCCR0A |= (1 << CTC0) | (1 << CS01);	//CTC Mode / Prescaler 8
	OCR0A = 125 - 1;						// 1000000 / 8 / 1000 = 125 -> 1000Hz
	TIMSK0 = (1<<OCIE0A);					//Enable Compare Interrupt
	GTCCR &= ~(1 << TSM);					//Timer starten
}

//1ms
ISR(TIMER0_COMPA_vect)
{
	static uint16_t cnt = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0, cnt5 = 0;
	cnt++;
	cnt2++;
	cnt3++;
	cnt4++;
	cnt5++;
	if(cnt4 > 10)	//10ms
	{
		cnt4=0;
		//buzzer_SyncTask();
	}
	if(cnt > 100)	//100ms
	{
		cnt = 0;
		display_SyncTask();
	}
	if(cnt2 > 1000)	//1000ms
	{
		cnt2 = 0;
		pump_vSyncTask();
		power_SyncTask();
	}
	if(cnt3 > 50)	//50ms
	{
		cnt3 = 0;
		button_SyncTask();
	}
	if(cnt5 > 5000)	//every 5s
	{
		cnt5 = 0;
		temp_updateTemp();
	}
}
