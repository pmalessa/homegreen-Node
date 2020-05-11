
#include "timer.hpp"

volatile uint32_t Timer::millis = 0;

void Timer::Init()
{
	//Init 1ms Timer, use Timer 0
	TCCR0A = 0x00;							//Register zuruecksetzen
	TCCR0A |= (1 << CTC0) | (1 << CS01);	//CTC Mode / Prescaler 8
	OCR0A = 125 - 1;						// 1000000 / 8 / 1000 = 125 -> 1000Hz
	TIMSK0 = (1<<OCIE0A);					//Enable Compare Interrupt
	GTCCR &= ~(1 << TSM);					//Timer starten
}

void Timer::Sleep()
{
	TIMSK0 = 0; //disable Timer0 interrupts
	//PRR |= (1 << PRTIM0);
}

void Timer::Wakeup()
{
	//PRR &= ~(1 << PRTIM0);
	TIMSK0 = (1<<OCIE0A); //enable Timer0 Compare Interrupt
}

uint32_t Timer::getMillis()
{
	return millis;
}

uint32_t Timer::getCurrentRuntime()	//get current Runtime since last Reset in 0.1 Hours steps
{
	return (millis/1000/60/6); //0.1 hours
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
