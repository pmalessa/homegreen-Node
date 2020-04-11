
#include "timer.hpp"

volatile uint32_t Timer::millis = 0;

void Timer::Init()
{
	//Init 1ms Timer, use Timer 0
	TCCR0A = 0x00;							//Register zuruecksetzen
	TCCR0A |= (1 << CTC0) | (1 << CS01)| (1 << CS00);	//CTC Mode / Prescaler 64
	OCR0A = 125 - 1;						// 1000000 / 8 / 1000 = 125 -> 1000Hz
	TIMSK0 = (1<<OCIE0A);					//Enable Compare Interrupt
	GTCCR &= ~(1 << TSM);					//Timer starten
}

uint32_t Timer::getMillis()
{
	return millis;
}

void Timer::count()
{
	millis++;
}

//1ms
ISR(TIMER0_COMPA_vect)
{
	Timer::count();
}
