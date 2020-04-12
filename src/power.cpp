
#include "power.hpp"

uint16_t Power::LoadCounter = 0;
uint16_t Power::currentCapVoltage = 0;
uint8_t Power::adcStable = 0;
DeltaTimer Power::powerTimer;

void Power::Init() {
    
	PWR_IN_DDR |= _BV(PWR_IN_PIN);
    PWR_LOAD_DDR |= _BV(PWR_LOAD_PIN);
    PWR_5V_DDR &= ~(_BV(PWR_5V_PIN)); 							//digital input
	PWR_5V_PORT &= ~(_BV(PWR_5V_PIN));							//turn off internal pullup

    PWR_LOAD_PORT &= ~(_BV(PWR_LOAD_PIN));						//turn off load

	powerTimer.setTimeStep(10); //10 ms

	//Init 1kHz Clock Timer, use Timer 1
	TCCR1A = 0x00; TCCR1B = 0x00; TCCR1C = 0x00;		//Register zuruecksetzen
	TCCR1A |= _BV(COM1A0);							//Toggle OC1A / CTC Mode OCR1A / Prescaler 8
	TCCR1B |= _BV(WGM12) | _BV(CS11);
	OCR1A = 125 - 1;									// 1000000 / 8 / 1000 = 125 -> 1000Hz
	TIMSK1 = 0;											//No Interrupts
	GTCCR &= ~_BV(TSM);								//Timer starten

	Power::Wakeup();
}

void Power::Wakeup()
{
	//PRR &= ~_BV(PRADC);
    ADCSRA = _BV(ADEN) | _BV(ADATE)| _BV(ADPS1) | _BV(ADPS0);	//Enable, Auto Trigger, DIV8
    ADCSRB = 0;													//Free running mode
	ADMUX = CHANNEL_1V1;										//measuring 1.1V Reference Voltage
	ADMUX |= _BV(REFS0);										//using VCC Reference
	ADCSRA |= _BV(ADSC);										//start conversion

	currentCapVoltage = ADC;	//scrap measurement

	adcStable = false;
}

void Power::Sleep()
{
	ADCSRA = 0;	//disable ADC
	//PRR |= _BV(PRADC);
}

bool Power::isAdcStable()
{
	return 1;
	//return (adcStable == 10);	//ADC is stable once 10 measurements are buffered
}

bool Power::isPowerConnected()	//check if the 5V Pin is high
{
	uint8_t state = PWR_5V_PINREG & _BV(PWR_5V_PIN);	//read 5V Pin
	if(state)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Power::isCapLow()	//return if last measured CurVol lower than Threshold
{
	uint16_t curVol = adc2vol();
	if(curVol < LOWVOLTAGE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void Power::setInputPower(uint8_t state)
{
	if (state == 1)
	{
		PWR_IN_PORT &= ~_BV(PWR_IN_PIN); 		//turn on PB
		TCCR1A &= ~_BV(COM1A0);				//disable Clock Signal
		//PRR |= _BV(PRTIM1);
	}
	else
	{
		PWR_IN_PORT |= _BV(PWR_IN_PIN);	//turn off PB
		//PRR &= ~_BV(PRTIM1);
		TCCR1A |= _BV(COM1A0);			//enable Clock Signal
	}
}

void Power::setLoad(uint8_t state)
{
	if(state == 1)
	{
		PWR_LOAD_PORT |= _BV(PWR_LOAD_PIN);		//turn on load
	}
	else
	{
		PWR_LOAD_PORT &= ~_BV(PWR_LOAD_PIN);	//turn off load
	}
}

void Power::run()
{
	if(powerTimer.isTimeUp())	//every 10ms
	{
		//currentCapVoltage = (uint16_t) (ALPHA * ADC + (1-ALPHA) * currentCapVoltage);	//EWMA Filtering of ADC Input
		currentCapVoltage = ADC;
	}
}
