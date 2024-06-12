
#include "timer.hpp"

volatile uint32_t Timer::millis = 0;

void Timer::Init()
{
	//Init 250ms Timer, use Timer 0
	PRR &= ~_BV(PRTIM0);	//enable Timer0
	TCCR0A = 0x00;						//Register zuruecksetzen
	TCCR0A |= _BV(CTC0) | _BV(CS02) | _BV(CS00);	//CTC Mode / Prescaler 1024
	OCR0A = 255;						// 1000000 / 1024 / 4 = 256-1 -> 0.25Hz -> 250ms
	TIMSK0 = _BV(OCIE0A);				//Enable Compare Interrupt

	//Init 1kHz Clock Timer, use Timer 1
	TCCR1A = 0x00; TCCR1B = 0x00; TCCR1C = 0x00;	//Register zuruecksetzen
	TCCR1A &= ~_BV(COM1A0);							//disable Clock Output
	TCCR1B |= _BV(WGM12) | _BV(CS11);				// CTC Mode OCR1A / Prescaler 8
	OCR1A = 125 - 1;								// 1000000 / 8 / 1000 = 125 -> 1000Hz
	TIMSK1 = _BV(OCIE1A);							//Enable Compare Interrupt
	PRR &= ~_BV(PRTIM1);							//dont shut off Timer1
	GTCCR &= ~_BV(TSM);					//Timer starten
}

void Timer::shortSleep(uint32_t ms)
{
	uint8_t state = TIMSK1;	//save prev. state
	TIMSK1 = _BV(OCIE1A);
	set_sleep_mode(SLEEP_MODE_IDLE);	//Sleep mode Idle
	for(uint32_t i=0;i<ms;i++)
	{
		asm("wdr"); //reset watchdog
		cli();									//disable interrupts
		sleep_enable();							//enable sleep
		sei();									//enable interrupts
		sleep_cpu();							//sleep...
		/*zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz*/
		//waked up
		sleep_disable();						//disable sleep
	}
	TIMSK1 = state; //restore state
}

void Timer::Sleep()
{
	TIMSK1 = 0; //disable Timer1 interrupts
}

void Timer::Wakeup()
{
	TIMSK1 = _BV(OCIE1A); //enable Timer1 Compare Interrupt
}

uint32_t Timer::getMillis()
{
	return millis;
}

void Timer::count()
{
	millis++;
}
