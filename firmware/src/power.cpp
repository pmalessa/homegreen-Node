
#include "power.hpp"

uint16_t Power::LoadCounter = 0;
uint16_t Power::currentCapVoltage = 0;
DeltaTimer Power::powerTimer;
uint8_t Power::powerPinIntegrator = 0;
bool Power::powerPinState = 0;

void Power::Init() {
	EN_PB_DDR |= _BV(EN_PB_PIN);
	EN_PB_DDR |= _BV(EN_PB_PIN);
    EN_LOAD_DDR |= _BV(EN_LOAD_PIN);
    CHK_5V_DDR &= ~(_BV(CHK_5V_PIN)); 							//digital input
	CHK_5V_PORT &= ~(_BV(CHK_5V_PIN));							//turn off internal pullup

    EN_LOAD_PORT &= ~(_BV(EN_LOAD_PIN));						//turn off load

	//Todo: replace this once PCB Updated
	EN_CLK_DDR &= ~_BV(EN_CLK_PIN);	//set as input as its unused
	DDRB |= _BV(PB1);

	powerTimer.setTimeStep(10); //10 ms

	Power::Wakeup();
}

void Power::Wakeup()
{
	PRR &= ~_BV(PRADC);
    ADCSRA = _BV(ADEN) | _BV(ADATE)| _BV(ADPS1) | _BV(ADPS0);	//Enable, Auto Trigger, DIV8
    ADCSRB = 0;													//Free running mode
	ADMUX = CHANNEL_1V1;										//measuring 1.1V Reference Voltage
	ADMUX |= _BV(REFS0);										//using VCC Reference
	ADCSRA |= _BV(ADSC);										//start conversion

	currentCapVoltage = ADC;	//scrap measurement

	//initialize after wakeup
	powerPinState = CHK_5V_PINREG & _BV(CHK_5V_PIN);
	if(powerPinState){
		powerPinIntegrator = MAXIMUM;
	}else{
		powerPinIntegrator = 0;
	}
}

void Power::Sleep()
{
	ADCSRA = 0;	//disable ADC
	PRR |= _BV(PRADC);
}

bool Power::isPowerConnected()	//check if the 5V Pin is high
{
	return powerPinState;
}

bool Power::isCapLow()	//return if last measured CurVol lower than Threshold
{
	uint16_t curVol = adc2vol();
	if(curVol < POWER_LOWVOLTAGE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

bool Power::isCapFull()	//return if last measured CurVol higher than Cap Full High Threshold
{
	uint16_t curVol = adc2vol();
	if(curVol > POWER_CAPFULL)
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
	}
	else
	{
		EN_PB_PORT |= _BV(EN_PB_PIN);	//turn off PB
		TCCR1A |= _BV(COM1A0);			//enable Clock Signal
	}
}

void Power::setLoad(uint8_t state)
{
	if(state == 1)
	{
		EN_LOAD_PORT |= _BV(EN_LOAD_PIN);	//turn on load
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
		currentCapVoltage = ADC;

		//debouncing algorithm using integrator
		if ((CHK_5V_PINREG & _BV(CHK_5V_PIN)) == 0)	//read 5V Pin
		{
			if (powerPinIntegrator > 0)
			{
				powerPinIntegrator--;
			}
		}
		else if (powerPinIntegrator < MAXIMUM)
		{
			powerPinIntegrator++;
		}
		if (powerPinIntegrator == 0)
		{
			powerPinState = 0;
		}
		else if (powerPinIntegrator >= MAXIMUM)
		{
			powerPinState = 1;
			powerPinIntegrator = MAXIMUM;
		}
	}
}