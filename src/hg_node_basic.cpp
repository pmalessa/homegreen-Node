
#include "PLATFORM.h"
#include "display.hpp"
#include "timer.hpp"
#include "pump.hpp"
#include "power.hpp"
#include "data.hpp"
#include "temp.hpp"
#include "button.hpp"

typedef enum{
	STATE_BOOT,
	STATE_DISPLAY,
	STATE_CONFIG,
	STATE_SLEEP,
	STATE_WAKEUP,
	STATE_PUMPING,
	STATE_MAN_PUMPING,
	STATE_CHARGE_CAP
}state_t;
state_t state = STATE_BOOT;
uint8_t first = 1;
uint8_t wakeupTimeout = 0;
volatile uint8_t wdt_interrupt = 0;

ISR(WDT_vect) {
	wdt_interrupt = 1;
	Data::decCountdown(8);
}

void switchTo(state_t newstate)
{
	first = 1;
	state = newstate;
	Button::Clear();
}

void state_machine();

void anypress_callback()	//called if any Button pressed or released
{
	Display::ResetTimeout();
}

//m��p 1 otterdamoo <3

int main (void) {
	//watchdog init
	cli();
	uint8_t reset_flag = MCUSR;
	MCUSR = 0;
	MCUSR &= ~(1<<WDRF);								//unlock step 1
	WDTCSR = (1 << WDCE) | (1 << WDE);					//unlock step 2
	WDTCSR = (1 << WDIE) | (1 << WDP3) | (1 << WDP0); 	//Set to Interrupt Mode and "every 8 s"

	//power saving
//	PRR |= (1 << PRTWI) | (1 << PRTIM1);

	Timer::Init();
	Button::Init();
	Button::SetCallback(&anypress_callback);
	Pump::Init();
	Power::Init();
	Data::Init();
//	temp_init();

	//init debug pin
	BUZZER_DDR |= (1 << BUZZER_PIN);
	BUZZER_PORT |= (1 << BUZZER_PIN);

	sei();

	while(1)
	{
		state_machine();
		Display::Draw();
	}
}

void fade()
{
	Display::StartAnimation(Display::ANIMATION_FADE);
	while(!Display::IsAnimationDone())
	{
		Display::Draw();
	}
	Button::Clear();
}

void state_machine()
{
	static digit_t curdigit = DIGIT_INTERVAL;
	static uint32_t prev_countdown = 0;
	static Button::button_press press;
	static uint16_t chargeCounter = 0;

	switch (state) {
		case STATE_BOOT:
			if(first)
			{
				first = 0;
				Power::setInputPower(1);
				while(!Power::isAdcStable());			//wait till ADC is stable
				_delay_ms(100);
				if(Power::isPowerConnected() == true)	//if PB connected
				{
					Display::Init();
					Display::StartAnimation(Display::ANIMATION_BOOT);
					while(!Display::IsAnimationDone())
					{
						Display::Draw();
					}
					switchTo(STATE_DISPLAY);
					break;
				}
				else									//if on BatPower
				{
					switchTo(STATE_SLEEP);
					break;
				}
			}
			break;
		case STATE_DISPLAY:
			if(first)
			{
				first = 0;
				Display::ResetTimeout();
				Display::SetValue(DIGIT_DURATION,Data::Get(Data::DATA_DURATION));
				Display::SetValue(DIGIT_INTERVAL,Data::Get(Data::DATA_INTERVAL));
				Display::SetValue(DIGIT_COUNTDOWN,Data::getCountdownDisplay());
			}
			if(Display::IsTimeout())	//Display Timeout reached
			{
				fade();
				switchTo(STATE_SLEEP);
				break;
			}
			if(Data::getCountdown() != prev_countdown)	//update Display
			{
				prev_countdown = Data::getCountdown();
				Display::SetValue(DIGIT_COUNTDOWN,Data::getCountdownDisplay());
			}
			if(Data::getCountdown() == 0)			//if countdown reached, switch to PUMPING
			{
				fade();
				Data::resetCountdown();									//reset Countdown
				Data::Set(Data::DATA_PUMP_DURATION,Data::Get(Data::DATA_DURATION));	//set Pump Duration to configured duration
				switchTo(STATE_PUMPING);
				break;
			}
			if(Power::isPowerLost()) //if power lost
			{
				switchTo(STATE_SLEEP);
				break;
			}

			if((Button::isPressed(Button::BUTTON_MAN) == Button::BUTTON_LONG_PRESSING))	//switch to MAN_PUMPING
			{
				Display::Clear();
				Display::SetByte(4,0x3F);	//O
				Display::SetByte(5,0x54);	//N
				fade();
				Data::Set(Data::DATA_PUMP_DURATION,DATA_PUMP_DURATION_DEFAULT);	//2.0min
				switchTo(STATE_MAN_PUMPING);								//switch to MAN_PUMPING
				break;
			}

			if((Button::isPressed(Button::BUTTON_SET) == Button::BUTTON_LONG_PRESSING))
			{
				fade();
				switchTo(STATE_CONFIG);										//switch to CONFIG
				break;
			}
			break;
		case STATE_CONFIG:
			if(first)
			{
				first = 0;
				Display::ResetTimeout();
				curdigit = DIGIT_INTERVAL;
				Display::EnableBlinking(curdigit);

			}
			Display::SetValue(DIGIT_DURATION,Data::Get(Data::DATA_DURATION));
			Display::SetValue(DIGIT_INTERVAL,Data::Get(Data::DATA_INTERVAL));
			Display::SetValue(DIGIT_COUNTDOWN,Data::getCountdownDisplay());

			if(Data::getCountdown() != prev_countdown)						//update Countdown
			{
				prev_countdown = Data::getCountdown();
				Display::SetValue(DIGIT_COUNTDOWN,Data::getCountdownDisplay());
				if(Power::isPowerLost()) //if power lost
				{
					switchTo(STATE_SLEEP);
					break;
				}
			}
			press = Button::isPressed(Button::BUTTON_PLUS);							//get Button Plus Press
			if(press == Button::BUTTON_LONG_PRESSING)								//if long Press
			{
				Display::DisableBlinking();
				while(Button::isPressed(Button::BUTTON_PLUS))						//while pressed, fast increment
				{
					Data::Increment((Data::data_type_t)curdigit);
					Display::SetValue(curdigit,Data::Get((Data::data_type_t)curdigit));
					Display::Draw();
					_delay_ms(100);
				}
				Display::EnableBlinking(curdigit);
			}
			else if(press == Button::BUTTON_SHORT_PRESS)							//if short press, increment one step
			{
				Data::Increment((Data::data_type_t)curdigit);
				Display::SetValue(curdigit,Data::Get((Data::data_type_t)curdigit));
			}
			press = Button::isPressed(Button::BUTTON_MINUS);
			if(press == Button::BUTTON_LONG_PRESSING)								//if long Press
			{
				Display::DisableBlinking();
				while(Button::isPressed(Button::BUTTON_MINUS))						//while pressed, fast decrement
				{
					Data::Decrement((Data::data_type_t)curdigit);
					Display::SetValue(curdigit,Data::Get((Data::data_type_t)curdigit));
					Display::Draw();
					_delay_ms(100);
				}
				Display::EnableBlinking(curdigit);
			}
			else if(press == Button::BUTTON_SHORT_PRESS)							//if short press, decrement one step
			{
				Data::Decrement((Data::data_type_t)curdigit);
				Display::SetValue(curdigit,Data::Get((Data::data_type_t)curdigit));
			}
			press = Button::isPressed(Button::BUTTON_SET);							//get Set Button Press
			if(press == Button::BUTTON_LONG_PRESSING || Display::IsTimeout())	//Long Press or IDLE timeout, Config done
			{
				Data::Save();												//save to EEPROM
				Display::DisableBlinking();
				fade();
				Data::resetCountdown();										//reset Countdown
				switchTo(STATE_DISPLAY);									//switch to State Display
				break;
			}
			else if(press == Button::BUTTON_SHORT_PRESS)							//Short SET Press, switch selected Digit
			{
				if(curdigit == DIGIT_DURATION)
				{
					curdigit = DIGIT_INTERVAL;
					Display::EnableBlinking(curdigit);
				}
				else
				{
					curdigit = DIGIT_DURATION;
					Display::EnableBlinking(curdigit);
				}
			}
			break;
		case STATE_SLEEP:
			if(first)
			{
				first = 0;
				Display::DeInit();
				Power::setInputPower(0);				//disable Powerbank
			}
			BUZZER_PORT &= ~(1 << BUZZER_PIN);
			//PRR |= (1 << PRADC);
		    set_sleep_mode(SLEEP_MODE_PWR_DOWN);	//Sleep mode: only wdt and pin interrupt
		    cli();									//disable interrupts
			sleep_enable();							//enable sleep
//			sleep_bod_disable();					//disable BOD for power save
			sei();									//enable interrupts
			sleep_cpu();							//sleep...
			/*zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz*/
			//waked up
			sleep_disable();						//disable sleep
			//PRR &= ~(1 << PRADC);

			sei();									//enable interrupts

			if(wdt_interrupt == 1)					//wdt interrupt wakeup
			{
				wdt_interrupt = 0;
				if(Data::getCountdown() == 0)		//if countdown reached
				{
					switchTo(STATE_WAKEUP);
				}
				if(chargeCounter > 0)
				{
					chargeCounter--;
				}
				else
				{
					Power::setInputPower(0);
				}
				if(Power::isPowerLow())
				{
					switchTo(STATE_CHARGE_CAP);
				}
			}
			else if(Button::isAnyPressed())			//button interrupt wakeup
			{
				switchTo(STATE_WAKEUP);
			}
			break;
		case STATE_WAKEUP:
			if(first)
			{
				first = 0;
				Power::setInputPower(1);
				wakeupTimeout = 5;	//try for 5 seconds
			}
			if(Power::isPowerConnected())
			{
				Power::setLoad(1);
				_delay_ms(150);
				Power::setLoad(0);
				_delay_ms(150);
				Power::setGracePeriod();	//ignore power lost for 2 seconds
				BUZZER_PORT |= (1 << BUZZER_PIN);
				//successfully woken up
				Display::Init();
				Display::StartAnimation(Display::ANIMATION_WAKE);
				while(!Display::IsAnimationDone())
				{
					Display::Draw();
					_delay_ms(30);
				}
				switchTo(STATE_DISPLAY);		//switch to Display State
			}
			else
			{
				if(wakeupTimeout)
				{
					wakeupTimeout--;
					Power::setLoad(1);
					_delay_ms(500);
					Power::setLoad(0);
					_delay_ms(500);
				}
				else
				{
					switchTo(STATE_SLEEP);	//unsuccessful, back to sleep, try 1min later?
				}
			}
			break;
		case STATE_CHARGE_CAP:
			if(first)
			{
				first = 0;
				Power::setInputPower(1);
				wakeupTimeout = 5;	//try for 5 seconds
			}
			if(Power::isPowerConnected())
			{
				Power::setLoad(1);
				_delay_ms(150);
				Power::setLoad(0);
				_delay_ms(150);
				Power::setGracePeriod();	//ignore power lost for 2 seconds
				BUZZER_PORT |= (1 << BUZZER_PIN);
				//successfully woken up
				Display::Init();
				Display::StartAnimation(Display::ANIMATION_WAKE);
				while(!Display::IsAnimationDone())
				{
					Display::Draw();
					Power::setLoad(1);
					_delay_ms(150);		//charging, producing load
					Power::setLoad(0);
					_delay_ms(900);
				}
				switchTo(STATE_SLEEP);		//switch to Sleep state
			}
			else
			{
				if(wakeupTimeout)
				{
					wakeupTimeout--;
					Power::setLoad(1);
					_delay_ms(500);
					Power::setLoad(0);
					_delay_ms(500);
				}
				else
				{
					switchTo(STATE_SLEEP);	//unsuccessful, back to sleep, try 1min later?
				}
			}
			break;
		case STATE_PUMPING:
		case STATE_MAN_PUMPING:
			if(first)
			{
				first = 0;
				_delay_ms(1000);							//wait for Cap to charge a bit
				Pump::Enable(Data::Get(Data::DATA_PUMP_DURATION));	//enable Pump for specified duration
				Display::StartAnimation(Display::ANIMATION_PUMP);
			}
			if(Pump::getCountdown() == 0)					//if pump duration reached, switch to Display State
			{
				Data::resetCountdown();						//reset Countdown
				fade();
				switchTo(STATE_DISPLAY);
				break;
			}
			Display::ResetTimeout(); 						//Display always on
			press = Button::isPressed(Button::BUTTON_MAN);
			if(press == Button::BUTTON_LONG_PRESSING)				//if Button MAN long pressed, disable Pump
			{
				Display::Clear();
				Display::SetByte(3,0x3F);	//O
				Display::SetByte(4,0x71);	//F
				Display::SetByte(5,0x71);	//F
				Pump::Disable();
				//Deltatimer 1500ms
				switchTo(STATE_DISPLAY);					//switch to Display State
				break;
			}

			press = Button::isPressed(Button::BUTTON_PLUS);
			if(press == Button::BUTTON_LONG_PRESSING)				//if Plus Button long pressed, fast increment
			{
				while(Button::isPressed(Button::BUTTON_PLUS))
				{
					Data::Increment(Data::DATA_PUMP_DURATION);
					Pump::Enable(Data::Get(Data::DATA_PUMP_DURATION));
					Display::SetValue(DIGIT_COUNTDOWN,Data::Get(Data::DATA_PUMP_DURATION));
					Display::Draw();
					_delay_ms(100);
				}
			}
			else if(press == Button::BUTTON_SHORT_PRESS)			//if short Press, one increment
			{
				Data::Increment(Data::DATA_PUMP_DURATION);
				Pump::Enable(Data::Get(Data::DATA_PUMP_DURATION));
				Display::SetValue(DIGIT_COUNTDOWN,Data::Get(Data::DATA_PUMP_DURATION));
			}

			press = Button::isPressed(Button::BUTTON_MINUS);
			if(press == Button::BUTTON_LONG_PRESSING)				//if Minus Button long pressed, fast decrement
			{
				while(Button::isPressed(Button::BUTTON_MINUS))
				{
					Data::Decrement(Data::DATA_PUMP_DURATION);
					Pump::Enable(Data::Get(Data::DATA_PUMP_DURATION));
					Display::SetValue(DIGIT_COUNTDOWN,Data::Get(Data::DATA_PUMP_DURATION));
					Display::Draw();
					_delay_ms(100);
				}
			}
			else if(press == Button::BUTTON_SHORT_PRESS)			//if short pressed, one decrement
			{
				Data::Decrement(Data::DATA_PUMP_DURATION);
				Pump::Enable(Data::Get(Data::DATA_PUMP_DURATION));
				Display::SetValue(DIGIT_COUNTDOWN,Data::Get(Data::DATA_PUMP_DURATION));
			}
			/*
			if(power_isPowerLow()) //if power lost
			{
				pump_disable();
				switchTo(STATE_SLEEP);
				break;
			}
			*/
			break;
	}
}
