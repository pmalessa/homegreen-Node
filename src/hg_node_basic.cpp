
#include "PLATFORM.h"
#include "display.hpp"
#include "timer.hpp"
#include "pump.hpp"
#include "power.hpp"
#include "data.hpp"
#include "temp.hpp"
#include "button.hpp"
extern "C"
{
  #include "driver/light_ws2812.h"
}


typedef enum{
	STATE_BOOT,
	STATE_DISPLAY,
	STATE_CONFIG,
	STATE_SLEEP,
	STATE_WAKEUP,
	STATE_PUMPING,
	STATE_MAN_PUMPING
}state_t;
state_t state = STATE_BOOT;
uint8_t first = 1;
uint8_t wakeupTimeout = 0;
volatile uint8_t wdt_interrupt = 0;
struct cRGB led[1];
DeltaTimer buttonStepTimer;

ISR(WDT_vect) {
	wdt_interrupt = 1;
	Data::decCountdown(8);
	led[0].r = 0x10;
	ws2812_setleds(led,1);
	_delay_ms(20);
	led[0].r = 0x00;
	ws2812_setleds(led,1);

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

//m��p 1 otterdamoo <3

int main (void) {
	//watchdog init
	cli();
	uint8_t reset_flag = MCUSR;
	UNUSED(reset_flag);
	MCUSR = 0;
	MCUSR &= ~(1<<WDRF);								//unlock step 1
	WDTCSR = (1 << WDCE) | (1 << WDE);					//unlock step 2
	WDTCSR = (1 << WDIE) | (1 << WDP3) | (1 << WDP0); 	//Set to Interrupt Mode and "every 8 s"

	led[0].r = 0;led[0].g = 0;led[0].b = 0;

	//power saving
//	PRR |= (1 << PRTWI) | (1 << PRTIM1);

	Timer::Init();
	Button::Init();
	Button::SetCallback(&anypress_callback);
	Pump::Init();
	Power::Init();
	Data::Init();
	Display::Init();
//	temp_init();

	buttonStepTimer.setTimeStep(100); //set step of long press

	sei();

	while(1)
	{
		state_machine();
		Display::Draw();
		Power::run();
		Button::run();
		Pump::run();
		_delay_ms(10);
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
	static uint8_t wakeReason = 0;
	static uint8_t currentPump = 0;

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
				_delay_ms(100);
				if(Power::isPowerConnected())	//if PB connected
				{
					led[0].g = 0x10;
					ws2812_setleds(led,1);
					_delay_ms(100);
					led[0].g = 0x00;
					ws2812_setleds(led,1);
					_delay_ms(100);
					led[0].g = 0x10;
					ws2812_setleds(led,1);
					_delay_ms(100);
					led[0].g = 0x00;
					ws2812_setleds(led,1);
					_delay_ms(100);
					Display::Init();
					Display::StartAnimation(Display::ANIMATION_BOOT);
				}
				else							//if no PB connected
				{
					led[0].b = 0x10;
					ws2812_setleds(led,1);
					_delay_ms(100);
					led[0].b = 0x00;
					ws2812_setleds(led,1);
					_delay_ms(100);
					led[0].b = 0x10;
					ws2812_setleds(led,1);
					_delay_ms(100);
					led[0].b = 0x00;
					ws2812_setleds(led,1);
					_delay_ms(100);
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
				Data::resetCountdown();				//reset Countdown
				fade();
				switchTo(STATE_PUMPING);
				break;
			}
			if(Power::isPowerLost()) 	//if power lost
			{
				switchTo(STATE_SLEEP);
				break;
			}
			if((Button::isPressed(Button::BUTTON_MAN) == Button::BUTTON_LONG_PRESS))	//switch to MAN_PUMPING
			{
				Display::Clear();
				Display::SetByte(4,0x3F);	//O
				Display::SetByte(5,0x54);	//N
				fade();
				switchTo(STATE_MAN_PUMPING);								//switch to MAN_PUMPING
				break;
			}
			if((Button::isPressed(Button::BUTTON_SET) == Button::BUTTON_LONG_PRESS))
			{
				fade();
				switchTo(STATE_CONFIG);										//switch to CONFIG
				break;
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
				Display::ResetTimeout();
				curdigit = DIGIT_INTERVAL;
				Display::EnableBlinking(curdigit);
				Display::SetValue(DIGIT_DURATION,Data::Get(Data::DATA_DURATION));
				Display::SetValue(DIGIT_INTERVAL,Data::Get(Data::DATA_INTERVAL));
				Display::SetValue(DIGIT_COUNTDOWN,Data::getCountdownDisplay());
			}
			if(Power::isPowerLost()) //if power lost
			{
				Data::Save();
				switchTo(STATE_SLEEP);
				break;
			}
			if(Data::getCountdown() != prev_countdown)	//update Countdown
			{
				prev_countdown = Data::getCountdown();
				Display::SetValue(DIGIT_COUNTDOWN,Data::getCountdownDisplay());
			}
			press = Button::isPressed(Button::BUTTON_PLUS);						//get Button Plus Press
			if(press == Button::BUTTON_LONG_PRESS)								//if long Press
			{
				if(buttonStepTimer.isTimeUp())
				{
					Button::clearOtherThan(Button::BUTTON_PLUS);
					Data::Increment((Data::data_type_t)curdigit);
					Display::SetValue(curdigit,Data::Get((Data::data_type_t)curdigit));
				}
			}
			else if(press == Button::BUTTON_SHORT_PRESS)							//if short press, increment one step
			{
				Data::Increment((Data::data_type_t)curdigit);
				Display::SetValue(curdigit,Data::Get((Data::data_type_t)curdigit));
			}
			
			press = Button::isPressed(Button::BUTTON_MINUS);
			if(press == Button::BUTTON_LONG_PRESS)								//if long Press
			{
				if(buttonStepTimer.isTimeUp())
				{
					Button::clearOtherThan(Button::BUTTON_MINUS);
					Data::Decrement((Data::data_type_t)curdigit);
					Display::SetValue(curdigit,Data::Get((Data::data_type_t)curdigit));
				}
			}
			else if(press == Button::BUTTON_SHORT_PRESS)							//if short press, decrement one step
			{
				Data::Decrement((Data::data_type_t)curdigit);
				Display::SetValue(curdigit,Data::Get((Data::data_type_t)curdigit));
			}
			press = Button::isPressed(Button::BUTTON_SET);							//get Set Button Press
			if(press == Button::BUTTON_LONG_PRESS || Display::IsTimeout())	//Long Press or IDLE timeout, Config done
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
			}
			DEBUG1_PORT &= ~(1 << DEBUG1_PIN);		//Turn off Debug Port
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
					wakeReason = 0; //Reason Wakeup for Countdown
					switchTo(STATE_WAKEUP);
				}
				if(Power::isPowerLow())
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
			/** Wakeup State
			 * Run multiple Load cycles to wakeup Powerbank
			 * once successful, check Wakeup Reason:
			 *     Countdown reached -> Pump State
			 *     Power low -> Charge Cap and Sleep State
			 *     Button -> Display State
			 * if not successful, back to sleep and try again
			 */
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
				DEBUG1_PORT |= (1 << DEBUG1_PIN);	//Set Debug Pin
				//successfully woken up
				Display::Init();
				switch (wakeReason)
				{
				case 0: //Countdown
					Display::StartAnimation(Display::ANIMATION_WAKE);
					while(!Display::IsAnimationDone())
					{
						Display::Draw();
						_delay_ms(30);
					}
					switchTo(STATE_PUMPING);		//switch to Pump State
					break;
				case 1: //Charging
					Display::StartAnimation(Display::ANIMATION_CHARGE);
					while(!Display::IsAnimationDone())
					{
						Display::Draw();
						_delay_ms(30);
					}
					Display::StopAnimation();
					switchTo(STATE_SLEEP);		//switch to Sleep State
					break;
				case 2: //Button
					Display::StartAnimation(Display::ANIMATION_WAKE);
					while(!Display::IsAnimationDone())
					{
						Display::Draw();
						_delay_ms(30);
					}
					switchTo(STATE_DISPLAY);		//switch to Display State
					break;
				default:
					break;
				}
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
			/** Pump State
			 * Enable Pump and start Countdown
			 * if Countdown reached -> Display State
			 * if Man long Press -> stop Pump, Display State
			 * if Plus/Minus Short/Long Press -> Increment/Decrement Countdown
			 */
			if(first)
			{
				first = 0;
				_delay_ms(1000);								//wait for Cap to charge a bit
				Pump::Enable(Data::Get(Data::DATA_DURATION)*6);	//enable Pump for specified duration
				Display::StartAnimation(Display::ANIMATION_PUMP);
			}
			if(Pump::getCountdown() == 0)					//if pump duration reached, switch to Display State
			{
				Display::StopAnimation();
				Data::resetCountdown();						//reset Countdown
				fade();
				switchTo(STATE_DISPLAY);
				break;
			}
			if(Pump::isHubConnected())
			{
				led[0].g = 0x10;
				ws2812_setleds(led,1);
				_delay_ms(20);
				led[0].g = 0x00;
				ws2812_setleds(led,1);
			}
			else
			{
				led[0].b = 0x10;
				ws2812_setleds(led,1);
				_delay_ms(20);
				led[0].b = 0x00;
				ws2812_setleds(led,1);
			}
			Display::ResetTimeout(); 						//Display always on
			press = Button::isPressed(Button::BUTTON_MAN);
			if(press == Button::BUTTON_LONG_PRESS)				//if Button MAN long pressed, disable Pump
			{
				Display::StopAnimation();
				Display::Clear();
				Display::SetByte(3,0x3F);	//O
				Display::SetByte(4,0x71);	//F
				Display::SetByte(5,0x71);	//F
				Pump::Disable();
				fade();
				switchTo(STATE_DISPLAY);					//switch to Display State
				break;
			}
			else if(press == Button::BUTTON_SHORT_PRESS)			//if short Press, switch pump
			{
				Pump::selectPump((currentPump++)%3);
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
			if(Power::isPowerLow()) //if power lost
			{
				Pump::Disable();
				Display::StopAnimation();
				switchTo(STATE_SLEEP);
				break;
			}
			break;
	}
}