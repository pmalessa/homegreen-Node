
#include "PLATFORM.h"
#include "display.hpp"
#include "timer.hpp"
#include "pump.hpp"
#include "power.hpp"
#include "data.hpp"
#include "temp.hpp"
#include "button.hpp"
#include "Led.hpp"

/**
 * Homegreen Node Basic Firmware
 * -----------------------------
 * Power Consumption while Sleep:
 * - CPU in IDLE Mode, 1MHz Clock still driving Timer and IO
 * - Timer0 off (maybe using for 1sec wakeup later as wdt inaccurate?), Timer1 on 1kHz for Voltage Doubler
 * 
 **/



typedef enum{
	STATE_BOOT,
	STATE_DISPLAY,
	STATE_CONFIG,
	STATE_SLEEP,
	STATE_WAKEUP,
	STATE_PUMPING,
	STATE_MAN_PUMPING,
	STATE_INFO
}state_t;
state_t state = STATE_BOOT;

uint8_t first = 1;
uint8_t loadCounter = 0;
uint8_t wakeReason = 0;
volatile uint8_t wdt_interrupt = 0;
DeltaTimer buttonStepTimer, runtimeTimer;

ISR(WDT_vect) {
	wdt_interrupt = 1;
	Data::decCountdown(8);
}

void switchTo(state_t newstate)
{
	first = 1;
	state = newstate;
	Display::StopAnimation();
	Button::Clear();
}

void state_machine();

void anypress_callback()	//called if any Button pressed or released
{
	Display::ResetTimeout();
}

//mööp 1 otterdamoo <3

int main (void) {
	//watchdog init
	cli();
	uint8_t reset_flag = MCUSR;
	if(reset_flag & (1<< PORF))
	{
		//Power on Reset
	}
	else if(reset_flag & (1<< BORF))
	{
		//BrownOut Reset - no real reset, switch to charging
		state = STATE_WAKEUP;
		wakeReason = 2; //Charging
	}
	else if(reset_flag & (1<< EXTRF))
	{
		//External Reset
	}
	else if(reset_flag & (1<< WDRF))
	{
		//Watchdog Reset
	}

	MCUSR = 0;
	MCUSR &= ~(1<<WDRF);								//unlock step 1
	WDTCSR = (1 << WDCE) | (1 << WDE);					//unlock step 2
	WDTCSR = (1 << WDIE) | (1 << WDP3) | (1 << WDP0); 	//Set to Interrupt Mode and "every 8 s"

	Led::Init();
	Timer::Init();
	Button::Init();
	Button::SetCallback(&anypress_callback);
	Pump::Init();
	Power::Init();
	Data::Init();
	Display::Init();
	Temp::Init();

	DEBUG1_DDR |= _BV(DEBUG1_PIN);

	buttonStepTimer.setTimeStep(100); //set step of long press
	runtimeTimer.setTimeStep(86400000); //24h, 1 day

	sei();

	while(1)
	{
		state_machine();
		Display::Draw();
		Power::run();
		Button::run();
		Pump::run();
		Temp::run();
		if(runtimeTimer.isTimeUp())
		{
			Data::Set(Data::DATA_TOTAL_RUNTIME,Data::Get(Data::DATA_TOTAL_RUNTIME)+10);
			Data::Save();
		}
		Timer::shortSleep(10);
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
	static uint8_t currentPump = 0;
	static bool hubConnected = false;

	switch (state) {
		case STATE_BOOT:
			/** Boot State
			 * if PB connected -> Display State
			 * else -> Sleep State
			 */
			if(first)
			{
				first = 0;
				Power::setInputPower(1);
				Led::Blink(2,100);
				if(Power::isPowerConnected())	//if PB connected
				{
					Display::Init();
					Display::StartAnimation(Display::ANIMATION_BOOT);
				}
				else							//if no PB connected
				{
					Led::Blink(1,100);
					switchTo(STATE_SLEEP);		//-> Sleep State
					break;
				}
			}
			if(Display::IsAnimationDone())	//if Animation done
			{
				Display::StopAnimation();
				switchTo(STATE_DISPLAY);	//-> Display State
			}
			break;
		case STATE_DISPLAY:
			/** Display State
			 * if Display Timeout -> Sleep State
			 * if Long Config Press -> Config State
			 * if Pump Countdown reached -> Pump State
			 * if Power Lost -> Sleep State
			 * if Man Long Press -> Man Pump State
			 */
			if(first)
			{
				first = 0;
				hubConnected = Pump::isHubConnected();
				Display::ResetTimeout();
				Display::SetValue(DIGIT_DURATION,Data::Get(Data::DATA_DURATION1));
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
				Display::ResetTimeout();
				Data::resetCountdown();				//reset Countdown
				fade();
				switchTo(STATE_PUMPING);
				break;
			}
			if(!Power::isPowerConnected()) 	//if power lost
			{
				switchTo(STATE_SLEEP);
				break;
			}
			if(Button::isPressed(Button::BUTTON_MAN) == Button::BUTTON_LONG_PRESS)	//switch to MAN_PUMPING
			{
				Display::ResetTimeout();
				Display::Clear();
				Display::SetByte(4,0x3F);	//O
				Display::SetByte(5,0x54);	//N
				fade();
				switchTo(STATE_MAN_PUMPING);								//switch to MAN_PUMPING
				break;
			}
			press = Button::isPressed(Button::BUTTON_SET);						//get Button Set Press
			if(press == Button::BUTTON_LONG_PRESS)								//if long Press
			{
				Display::ResetTimeout();
				fade();
				switchTo(STATE_CONFIG);										//switch to CONFIG
				break;
			}
			else if(press == Button::BUTTON_SHORT_PRESS)				//if short press, switch Pump Display
			{
				Display::ResetTimeout();
				if(hubConnected)
				{
					currentPump++;
					if(currentPump>2)currentPump = 0;	//0..2
				}
			}
			break;
		case STATE_CONFIG:
			/**
			 * Config State
			 * if long Set press -> save and Display State
			 * if Plus/Minus Long/Short Press -> Increment/Decrement current Value
			 * if short Set Press -> switch Value
			 * if Display Timeout -> Display State
			 * if Power Lost -> Sleep State
			 */
			if(first)
			{
				first = 0;
				curdigit = DIGIT_INTERVAL;
				currentPump = 0;
				prev_countdown = 0;
				Display::ResetTimeout();
				Display::EnableBlinking(curdigit);
				Display::SetValue(DIGIT_DURATION,Data::Get((Data::data_type_t)(Data::DATA_DURATION1+currentPump)));
				Display::SetValue(DIGIT_INTERVAL,Data::Get(Data::DATA_INTERVAL));
				Display::SetByte(4,0x73);	//P
				Display::SetByte(5,Display::numToByte(currentPump+1));	//1..3
			}
			if(!Power::isPowerConnected()) //if power lost
			{
				Data::Save();
				switchTo(STATE_SLEEP);
				break;
			}
			press = Button::isPressed(Button::BUTTON_PLUS);	//get Button Plus Press
			if(press == Button::BUTTON_LONG_PRESS)			//if long Press
			{
				if(buttonStepTimer.isTimeUp())
				{
					Button::clearOtherThan(Button::BUTTON_PLUS);
					Display::ResetTimeout();
					Display::ResetBlinkCounter();
					switch (curdigit)
					{
					case DIGIT_INTERVAL:
						Data::Increment((Data::data_type_t)DIGIT_INTERVAL);
						Display::SetValue(DIGIT_INTERVAL,Data::Get((Data::data_type_t)DIGIT_INTERVAL));
						break;
					case DIGIT_DURATION:
						Data::Increment((Data::data_type_t)(DIGIT_DURATION+currentPump));
						Display::SetValue(DIGIT_DURATION,Data::Get((Data::data_type_t)(DIGIT_DURATION+currentPump)));
						break;					
					case DIGIT_COUNTDOWN:
						currentPump++;
						if(currentPump >2)currentPump=0; //0..2
						Display::SetValue(DIGIT_DURATION,Data::Get((Data::data_type_t)(DIGIT_DURATION+currentPump)));
						Display::SetByte(5,Display::numToByte(currentPump+1));	//1..3
						break;
					}
				}
			}
			else if(press == Button::BUTTON_SHORT_PRESS)							//if short press, increment one step
			{
				Display::ResetBlinkCounter();
				switch (curdigit)
				{
				case DIGIT_INTERVAL:
					Data::Increment((Data::data_type_t)DIGIT_INTERVAL);
					Display::SetValue(DIGIT_INTERVAL,Data::Get((Data::data_type_t)DIGIT_INTERVAL));
					break;
				case DIGIT_DURATION:
					Data::Increment((Data::data_type_t)(DIGIT_DURATION+currentPump));
					Display::SetValue(DIGIT_DURATION,Data::Get((Data::data_type_t)(DIGIT_DURATION+currentPump)));
					break;					
				case DIGIT_COUNTDOWN:
					currentPump++;
					if(currentPump >2)currentPump=0; //0..2
					Display::SetValue(DIGIT_DURATION,Data::Get((Data::data_type_t)(DIGIT_DURATION+currentPump)));
					Display::SetByte(5,Display::numToByte(currentPump+1));	//1..3
					break;
				}
			}
			
			press = Button::isPressed(Button::BUTTON_MINUS);	//get Button Minus Press
			if(press == Button::BUTTON_LONG_PRESS)				//if long Press
			{
				if(buttonStepTimer.isTimeUp())
				{
					Button::clearOtherThan(Button::BUTTON_MINUS);
					Display::ResetTimeout();
					Display::ResetBlinkCounter();
					switch (curdigit)
					{
					case DIGIT_INTERVAL:
						Data::Decrement((Data::data_type_t)DIGIT_INTERVAL);
						Display::SetValue(DIGIT_INTERVAL,Data::Get((Data::data_type_t)DIGIT_INTERVAL));
						break;
					case DIGIT_DURATION:
						Data::Decrement((Data::data_type_t)(DIGIT_DURATION+currentPump));
						Display::SetValue(DIGIT_DURATION,Data::Get((Data::data_type_t)(DIGIT_DURATION+currentPump)));
						break;					
					case DIGIT_COUNTDOWN:
						if(currentPump == 0)currentPump=3;
						currentPump--; //0..2
						Display::SetValue(DIGIT_DURATION,Data::Get((Data::data_type_t)(DIGIT_DURATION+currentPump)));
						Display::SetByte(5,Display::numToByte(currentPump+1));	//1..3
						break;
					}
				}
			}
			else if(press == Button::BUTTON_SHORT_PRESS)							//if short press, decrement one step
			{
				Display::ResetBlinkCounter();
				switch (curdigit)
				{
				case DIGIT_INTERVAL:
					Data::Decrement((Data::data_type_t)DIGIT_INTERVAL);
					Display::SetValue(DIGIT_INTERVAL,Data::Get((Data::data_type_t)DIGIT_INTERVAL));
					break;
				case DIGIT_DURATION:
					Data::Decrement((Data::data_type_t)(DIGIT_DURATION+currentPump));
					Display::SetValue(DIGIT_DURATION,Data::Get((Data::data_type_t)(DIGIT_DURATION+currentPump)));
					break;					
				case DIGIT_COUNTDOWN:
					if(currentPump == 0)currentPump=3;
					currentPump--;	//0..2
					Display::SetValue(DIGIT_DURATION,Data::Get((Data::data_type_t)(DIGIT_DURATION+currentPump)));
					Display::SetByte(5,Display::numToByte(currentPump+1));	//1..3
					break;
				}
			}
			press = Button::isPressed(Button::BUTTON_SET);							//get Set Button Press
			if((press == Button::BUTTON_LONG_PRESS) || Display::IsTimeout())	//Long Press or IDLE timeout, Config done
			{
				Data::Save();												//save to EEPROM
				Display::DisableBlinking();
				Data::resetCountdown();										//reset Countdown
				fade();
				switchTo(STATE_DISPLAY);									//switch to State Display
				break;
			}
			else if(press == Button::BUTTON_SHORT_PRESS)							//Short SET Press, switch selected Digit
			{
				switch (curdigit)
				{
				case DIGIT_INTERVAL:
					curdigit = DIGIT_DURATION;
					break;
				case DIGIT_DURATION:
					if(hubConnected)
					{
						curdigit = DIGIT_COUNTDOWN;
					}
					else
					{
						curdigit = DIGIT_INTERVAL;
					}
					break;
				case DIGIT_COUNTDOWN:
					curdigit = DIGIT_INTERVAL;
					break;
				}
				Display::EnableBlinking(curdigit);
			}
			press = Button::isPressed(Button::BUTTON_MAN);							//get Button MAN Press
			if(press == Button::BUTTON_LONG_PRESS)	//if long Press
			{
				Display::DisableBlinking();
				Display::ResetTimeout();
				fade();
				switchTo(STATE_INFO);		//switch to State Info
				break;
			}
			break;
		case STATE_SLEEP:
			/** Sleep State
			 * Sleep and wait till next watchdog interrupt or button press.
			 * wdt interrupt: 
			 *     if countdown reached -> Wakeup State
			 *     if Power Low -> Charge State
			 *     else continue sleeping
			 * button interrupt -> Wakeup state
			 */
			if(first)
			{
				first = 0;
				Display::DeInit();					//DeInit Display
				Power::setInputPower(0);			//disable Powerbank
				Power::disableSolarCharger(false);	//reenable Solar Charger
			}
			Temp::Sleep();
			Power::Sleep();
			Timer::Sleep();
			DEBUG1_PORT &= ~(_BV(DEBUG1_PIN));
		    set_sleep_mode(SLEEP_MODE_IDLE);	//Sleep mode Idle: using Timer Clock for Voltage Doubler
			wdt_interrupt = 0;						//clear open interrupts
		    cli();									//disable interrupts
			sleep_enable();							//enable sleep
//			sleep_bod_disable();					//disable BOD for power save
			sei();									//enable interrupts
			sleep_cpu();							//sleep...
			/*zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz*/
			//waked up
			sleep_disable();						//disable sleep
			DEBUG1_PORT |= (_BV(DEBUG1_PIN));
			Timer::Wakeup();
			Power::Wakeup();
			sei();									//enable interrupts

			if(wdt_interrupt == 1)					//wdt interrupt wakeup
			{
				wdt_interrupt = 0;
				Led::Blink(1,20);
				if(Data::getCountdown() == 0)		//if countdown reached
				{
					wakeReason = 0; //Reason Wakeup for Countdown
					switchTo(STATE_WAKEUP);
				}
				if(Power::isCapLow())
				{
					wakeReason = 1; //Reason Wakeup for Charging
					switchTo(STATE_WAKEUP);
				}
			}
			else if(Button::isAnyPressed())			//button interrupt wakeup
			{
				wakeReason = 2; //Reason Wakeup for Button
				switchTo(STATE_WAKEUP);
			}
			break;
		case STATE_WAKEUP:
			if(first)
			{
				first = 0;
				loadCounter = 4;	//try for 20 times
				Led::On();
				Power::disableSolarCharger(true);	//disable Solar Charger and wait
				Timer::shortSleep(1000);	//TODO: remove
				Power::setLoad(1);
				Timer::shortSleep(200);
				Power::setLoad(0);
				Timer::shortSleep(200);
				Power::setInputPower(1);
			}
			if(Power::isPowerConnected())
			{
				//successfully woken up
				Led::Off();
				switch (wakeReason)
				{
				case 0: //Countdown
					Display::Init();
					Temp::Wakeup();	//only wakeup if 5V available
					Display::StartAnimation(Display::ANIMATION_WAKE);
					while(!Display::IsAnimationDone())
					{
						Display::Draw();
						Timer::shortSleep(30);
					}
					switchTo(STATE_PUMPING);		//switch to Pump State
					break;
				case 1: //Charging
					Led::fadeAnimation(); //Led Charge Animation
					switchTo(STATE_SLEEP);		//switch to Sleep State
					break;
				case 2: //Button
					Display::Init();
					Temp::Wakeup();	//only wakeup if 5V available
					Display::StartAnimation(Display::ANIMATION_WAKE);
					while(!Display::IsAnimationDone())
					{
						Display::Draw();
						Timer::shortSleep(30);
					}
					switchTo(STATE_DISPLAY);		//switch to Display State
					break;
				default:
					break;
				}
			}
			else
			{
				if(loadCounter)
				{
					loadCounter--;
					Timer::shortSleep(250);
				}
				else
				{
					Power::setInputPower(0);
					Led::Off();
					Timer::shortSleep(2000);
					switchTo(STATE_WAKEUP);	//unsuccessful, back to sleep, try 1min later?
				}
			}
			break;
		case STATE_PUMPING:
		case STATE_MAN_PUMPING:
			/** Pump State
			 * Enable Pump and start Countdown
			 * if Countdown reached -> Display State
			 * if Man long Press -> stop Pump, Display State
			 * if Plus/Minus Short/Long Press -> Increment/Decrement Countdown
			 */
			if(first)
			{
				first = 0;
				Timer::shortSleep(1000);	//wait for Cap to charge a bit
				Display::StartAnimation(Display::ANIMATION_PUMP);
				hubConnected = Pump::isHubConnected();
				currentPump = 0;
				Pump::setCountdown(Data::Get(Data::data_type_t(Data::DATA_DURATION1+currentPump))*6);
				Pump::Start();		//enable Pump for specified duration
			}
			Display::ResetTimeout(); 						//Display always on
			if(Pump::getCountdown() == 0)					//if pump duration reached, switch to Display State
			{
				if(hubConnected)	//switch to next Pump
				{
					currentPump++;
					if (currentPump > 2) //done
					{
						Display::StopAnimation();
						Data::resetCountdown();						//reset Countdown
						fade();
						switchTo(STATE_DISPLAY);
						break;
					}
					Pump::setCurrentPump(currentPump);
					Pump::setCountdown(Data::Get(Data::data_type_t(Data::DATA_DURATION1+currentPump))*6);
				}
				else
				{
					Display::StopAnimation();
					Data::resetCountdown();						//reset Countdown
					fade();
					switchTo(STATE_DISPLAY);
					break;
				}
			}
			Led::Blink(1,20);

			press = Button::isPressed(Button::BUTTON_MAN);
			if(press == Button::BUTTON_LONG_PRESS)				//if Button MAN long pressed, disable Pump
			{
				Display::StopAnimation();
				Display::Clear();
				Display::SetByte(3,0x3F);	//O
				Display::SetByte(4,0x71);	//F
				Display::SetByte(5,0x71);	//F
				Pump::Stop();
				if(state == STATE_PUMPING)	//if it was the ordinary Pump cycle, reset countdown
				{
					Data::resetCountdown();
				}
				fade();
				switchTo(STATE_DISPLAY);					//switch to Display State
				break;
			}
			else if(press == Button::BUTTON_SHORT_PRESS)			//if short Press, switch pump
			{
				if(hubConnected)
				{
					currentPump++;
					if(currentPump >2) currentPump=0;
					Pump::setCurrentPump(currentPump);
				}
			}
			press = Button::isPressed(Button::BUTTON_PLUS);
			if(press == Button::BUTTON_LONG_PRESS)				//if Plus Button long pressed, fast increment
			{
				if(buttonStepTimer.isTimeUp())
				{
					Pump::Increment();
				}
			}
			else if(press == Button::BUTTON_SHORT_PRESS)			//if short Press, one increment
			{
				Pump::Increment();
			}

			press = Button::isPressed(Button::BUTTON_MINUS);
			if(press == Button::BUTTON_LONG_PRESS)				//if Minus Button long pressed, fast decrement
			{
				if(buttonStepTimer.isTimeUp())
				{
					Pump::Decrement();
				}
			}
			else if(press == Button::BUTTON_SHORT_PRESS)			//if short pressed, one decrement
			{
				Pump::Decrement();
			}
			if(!Power::isPowerConnected()) //if power lost
			{
				Pump::Stop();
				Display::StopAnimation();
				switchTo(STATE_SLEEP);
				break;
			}
			break;

		case STATE_INFO:
			static uint8_t infoState = 0;
			if(first)
			{
				first=0;
				Display::Clear();
			}

			press = Button::isPressed(Button::BUTTON_SET);
			if(press == Button::BUTTON_SHORT_PRESS)
			{
				Display::Clear();
				infoState++;
				break;
			}
			else if (press == Button::BUTTON_LONG_PRESS)
			{
				fade();
				switchTo(STATE_DISPLAY);
			}
			press = Button::isPressed(Button::BUTTON_MAN);
			if(press == Button::BUTTON_LONG_PRESS)
			{
				Display::ResetTimeout();
				Display::DisableBlinking();
				Data::setDefault();	//reset EEPROM
				Display::SetValue(DIGIT_DURATION,Data::Get(Data::DATA_DURATION1));
				Display::SetValue(DIGIT_INTERVAL,Data::Get(Data::DATA_INTERVAL));
				Data::resetCountdown();
				Display::SetValue(DIGIT_COUNTDOWN,Data::getCountdownDisplay());
				fade();
				switchTo(STATE_DISPLAY);
				break;
			}
			switch (infoState)
			{
			case 0:	//Temp
				Display::SetByte(0,0x78);	//small t
				Display::SetNegValue(1,Data::GetTemp(Data::DATA_CURRENT_TEMP));
				break;
			case 1: //Build Date
				//Build Date
				Display::SetByte(0,Display::numToByte(BUILD_DAY/10));
				Display::SetByte(1,Display::numToByte(BUILD_DAY%10) | DEC_DOT);
				Display::SetByte(2,Display::numToByte(BUILD_MONTH/10));
				Display::SetByte(3,Display::numToByte(BUILD_MONTH%10) | DEC_DOT);
				Display::SetByte(4,Display::numToByte(BUILD_YEAR/10));
				Display::SetByte(5,Display::numToByte(BUILD_YEAR%10));
				break;
			case 2:
				//Build Version
				Display::SetByte(0,0x1C); //v
				Display::SetByte(1,Display::numToByte(BUILD_VERSION_MAJOR) | DEC_DOT);
				Display::SetByte(2,Display::numToByte(BUILD_VERSION_MINOR) | DEC_DOT);
				if(BUILD_VERSION_PATCH > 9)
				{
					Display::SetByte(3,Display::numToByte(BUILD_VERSION_PATCH/10));
					Display::SetByte(4,Display::numToByte(BUILD_VERSION_PATCH%10));
				}
				else
				{
					Display::SetByte(3,Display::numToByte(BUILD_VERSION_PATCH));
				}
				break;
			case 3: //Current Runtime in 4 digit hour and 1 decimal point hour
				Display::Set4DigValue(0,Timer::getCurrentRuntime());
				Display::SetByte(5,0x74); //h
				break;
			case 4: //Total Runtime
				Display::Set4DigValue(0,Data::Get(Data::DATA_TOTAL_RUNTIME));
				Display::SetByte(5,0x5E); //d
				break;
			default:
				infoState = 0;	//return
				break;
			}
			break;
	}
}