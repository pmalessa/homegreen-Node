
#include "power.hpp"

uint16_t Power::LoadCounter = 0;
uint16_t Power::currentCapVoltage = 0;
uint8_t Power::adcStable = 0;
DeltaTimer Power::powerTimer;

void Power::Init() {
	EN_PB_DDR |= _BV(EN_PB_PIN);
    EN_LOAD_DDR |= _BV(EN_LOAD_PIN);
    CHK_5V_DDR &= ~(_BV(CHK_5V_PIN)); 							//digital input
	CHK_5V_PORT &= ~(_BV(CHK_5V_PIN));							//turn off internal pullup

    EN_LOAD_PORT &= ~(_BV(EN_LOAD_PIN));						//turn off load

	EN_CLK_DDR |=_BV(EN_CLK_PIN);	//maybe to remove if bridged

	powerTimer.setTimeStep(10); //10 ms

	//Init 1kHz Clock Timer, use Timer 1
	TCCR1A = 0x00; TCCR1B = 0x00; TCCR1C = 0x00;		//Register zuruecksetzen
	TCCR1A |= _BV(COM1A0);							//Toggle OC1A / CTC Mode OCR1A / Prescaler 8
	TCCR1B |= _BV(WGM12) | _BV(CS11);
	OCR1A = 125 - 1;									// 1000000 / 8 / 1000 = 125 -> 1000Hz
	TIMSK1 = 0;											//No Interrupts
	PRR &= ~_BV(PRTIM1);	//dont shut off Timer1
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
	uint8_t state = CHK_5V_PINREG & _BV(CHK_5V_PIN);	//read 5V Pin
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
		EN_PB_PORT &= ~_BV(EN_PB_PIN); 		//turn on PB
		TCCR1A &= ~_BV(COM1A0);				//disable Clock Signal
		//PRR |= _BV(PRTIM1);
	}
	else
	{
		EN_PB_PORT |= _BV(EN_PB_PIN);	//turn off PB
		//PRR &= ~_BV(PRTIM1);
		TCCR1A |= _BV(COM1A0);			//enable Clock Signal
	}
}

void Power::setLoad(uint8_t state)
{
	if(state == 1)
	{
		EN_LOAD_PORT |= _BV(EN_LOAD_PIN);		//turn on load
	}
	else
	{
		EN_LOAD_PORT &= ~_BV(EN_LOAD_PIN);	//turn off load
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
