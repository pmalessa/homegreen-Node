
#include "PLATFORM.h"
#include "display.hpp"
#include "timer.hpp"
#include "pump.hpp"
#include "power.hpp"
#include "data.hpp"
#include "button.hpp"
#include "Led.hpp"

/**
 * Homegreen Node Basic Firmware
 * -----------------------------
 * Timer 0 used as 1ms Timer in active mode, disabled in sleep
 * Timer 1 used as Clock Output for Voltage Doubler in Sleep Mode, off in active mode
 * 
 **/

//State Machine Typedef
typedef enum{
	STATE_BOOT,
	STATE_DISPLAY,
	STATE_CONFIG,
	STATE_SLEEP,
	STATE_CHARGING,
	STATE_WAKEUP,
	STATE_PUMPING,
	STATE_INFO,
	STATE_SHOW_ERROR
}state_t;
state_t state = STATE_BOOT;

typedef enum{
	WAKESTAGE_FIRSTCHECK,
	WAKESTAGE_SECONDWAKE
}wakestage_t;

typedef enum{
	WAKEREASON_COUNTDOWN,
	WAKEREASON_BUTTON,
	WAKEREASON_CHARGING
}wakereason_t;

//Global Variables
uint8_t first = 1;
uint8_t checkCounter = 3, tryCounter = 3;	//try 3 times every 5 seconds, then Error
wakestage_t wakeupStage = WAKESTAGE_FIRSTCHECK;
wakereason_t wakeReason = WAKEREASON_COUNTDOWN;
Data::statusBit_t status;
volatile uint8_t wdt_interrupt = 0;
DeltaTimer buttonStepTimer;
volatile uint32_t currentRuntime = 0;	//runtime in seconds since last reset
//Function Prototypes
void state_machine();

ISR(TIMER0_COMPA_vect) {	//250ms
	static uint8_t cnt = 0;
	static uint16_t totalRuntimeStep = 0;
	cnt++;
	if(cnt > 15)	//16x250ms = 4 seconds
	{
		cnt = 0;
		wdt_interrupt = 1;
		currentRuntime +=4;
		totalRuntimeStep +=4;
		if(totalRuntimeStep > 24*360) //0.1days = 2.4*3600 sec
		{
			totalRuntimeStep = 0;
			Data::Set(Data::DATA_TOTAL_RUNTIME,Data::Get(Data::DATA_TOTAL_RUNTIME)+1);
		}
	}
}

void switchTo(state_t newstate)
{
	first = 1;
	state = newstate;
	Display::StopAnimation();
	Button::Clear();
}

void anypress_callback()	//called if any Button pressed or released
{
	Display::ResetTimeout();
}

static void __attribute__((noreturn))
main_loop (void)
{
	while(1)
	{
		asm("wdr"); //reset watchdog
		state_machine();
		Display::Draw();
		Power::run();
		Button::run();
		Pump::run();
		Timer::shortSleep(10);	//10ms delay
	}
}

int main (void) {
	//watchdog init
	cli();
	asm("wdr"); //reset watchdog
	uint8_t reset_flag = MCUSR;
	UNUSED(reset_flag);

	MCUSR = 0;
	MCUSR &= ~(1<<WDRF);								//unlock step 1
	WDTCSR = (1 << WDCE) | (1 << WDE);					//unlock step 2
	WDTCSR = (1 << WDE) | (1 << WDP3) | (1 << WDP0); 	//Set to Reset Mode and "every 8 s"

	Led::Init();
	Timer::Init();
	Button::Init();
	Button::SetCallback(&anypress_callback);
	Pump::Init();
	Power::Init();
	Data::Init();
	Display::Init();

	buttonStepTimer.setTimeStep(100); //set step of long press

	sei();

	main_loop();
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
			Led::Blink(LED_GREEN,2,100);
			switchTo(STATE_SLEEP);		//-> Sleep State
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
				Data::SaveConfig();	//if savePending, save data
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
				switchTo(STATE_PUMPING);								//switch to PUMPING
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
				Data::SaveConfig();	//if savePending, save data
			}
			if(!Power::isPowerConnected()) //if power lost
			{
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
#ifdef FEATURE_PUMP_STRENGTH
					case DIGIT_PUMP_STRENGTH:
						break;
#endif
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
#ifdef FEATURE_PUMP_STRENGTH
				case DIGIT_PUMP_STRENGTH:
					Data::SetPumpStrength(currentPump,(Data::GetPumpStrength(currentPump)+1)%3);
					Display::SetByte(3,Display::numToByte(Data::GetPumpStrength(currentPump)));
					break;
#endif
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
#ifdef FEATURE_PUMP_STRENGTH
					case DIGIT_PUMP_STRENGTH:
						break;
#endif
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
#ifdef FEATURE_PUMP_STRENGTH
				case DIGIT_PUMP_STRENGTH:
					if(Data::GetPumpStrength(currentPump) == 0){Data::SetPumpStrength(currentPump,2);}
					else{Data::SetPumpStrength(currentPump,Data::GetPumpStrength(currentPump)-1);}
					Display::SetByte(3,Display::numToByte(Data::GetPumpStrength(currentPump)));
					break;
#endif
				}
			}
			press = Button::isPressed(Button::BUTTON_SET);					//get Set Button Press
			if((press == Button::BUTTON_LONG_PRESS))	//Long Press, Config done
			{
				Data::setSavePending();										//save to EEPROM
				Display::DisableBlinking();
				Data::resetCountdown();										//reset Countdown
				Data::SaveConfig();
				fade();
				switchTo(STATE_DISPLAY);									//switch to State Display
				break;
			}
			else if(Display::IsTimeout())	//IDLE timeout, dont save
			{
				Data::resetFromEEPROM();									//load old values from EEPROM
				Display::DisableBlinking();
				Data::resetCountdown();										//reset Countdown
				fade();
				switchTo(STATE_DISPLAY);									//switch to State Display
				break;
			}
			else if(press == Button::BUTTON_SHORT_PRESS)					//Short SET Press, switch selected Digit
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
#ifdef FEATURE_PUMP_STRENGTH
						curdigit = DIGIT_PUMP_STRENGTH;
						Display::SetByte(0,0x73);	//P
						Display::SetByte(1,0x6D);	//S
						Display::SetByte(2,Display::numToByte(0));
						Display::SetByte(3,Display::numToByte(Data::GetPumpStrength(currentPump)));
#else
						curdigit = DIGIT_INTERVAL;
#endif
					}
					break;
				case DIGIT_COUNTDOWN:
#ifdef FEATURE_PUMP_STRENGTH
					curdigit = DIGIT_PUMP_STRENGTH;
					Display::SetByte(0,0x73);	//P
					Display::SetByte(1,0x6D);	//S
					Display::SetByte(2,Display::numToByte(0));
					Display::SetByte(3,Display::numToByte(Data::GetPumpStrength(currentPump)));
					break;
				case DIGIT_PUMP_STRENGTH:
					curdigit = DIGIT_INTERVAL;
					Display::SetValue(DIGIT_INTERVAL,Data::Get((Data::data_type_t)DIGIT_INTERVAL));
					Display::SetValue(DIGIT_DURATION,Data::Get((Data::data_type_t)(DIGIT_DURATION+currentPump)));
					break;
#else
					curdigit = DIGIT_INTERVAL;
#endif
				}
#ifdef FEATURE_PUMP_STRENGTH
				if (curdigit == DIGIT_PUMP_STRENGTH)
				{
					Display::EnableBlinking(DIGIT_DURATION);
				}else{
					Display::EnableBlinking(curdigit);
				}
#else
				Display::EnableBlinking(curdigit);
#endif
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
				Display::Sleep();					//DeInit Display
				Led::Off(LED_BTN);
				if(Power::isPowerConnected() && Power::isCapNotFull())	//if Powerbank available and Cap not full
				{
					switchTo(STATE_CHARGING);							//charge cap before going to sleep
					break;
				}
				Power::setInputPower(0);			//disable Powerbank
			}
			Power::Sleep();
			Timer::Sleep();
		    set_sleep_mode(SLEEP_MODE_IDLE);		//Sleep mode Idle: using Timer Clock for Voltage Doubler
			wdt_interrupt = 0;						//clear open interrupts
		    cli();									//disable interrupts
			sleep_enable();							//enable sleep
			sei();									//enable interrupts
			sleep_cpu();							//sleep...
			/*zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz*/
			//waked up
			sleep_disable();						//disable sleep
			Timer::Wakeup();
			Power::Wakeup();
			sei();									//enable interrupts

			if(wdt_interrupt == 1)					//wdt interrupt wakeup
			{
				Data::decCountdown(8);
				wdt_interrupt = 0;
				if(Data::GetErrors())	//if any error bit set
				{
					Led::Blink(LED_RED,1,20);	//blink error
				}
				else
				{
					Led::Blink(LED_GREEN,1,20);	//blink okay
				}
				if(Data::getCountdown() == 0)	//if countdown reached
				{
					wakeReason = WAKEREASON_COUNTDOWN; //Reason Wakeup for Countdown
					switchTo(STATE_WAKEUP);
				}
				if(Power::isCapLow())
				{
					wakeReason = WAKEREASON_CHARGING; //Reason Wakeup for Charging
					switchTo(STATE_WAKEUP);
				}
			}
			else if(Button::isAnyPressed())	//button interrupt wakeup
			{
				wakeReason = WAKEREASON_BUTTON; //Reason Wakeup for Button
				switchTo(STATE_WAKEUP);
			}
			break;
		case STATE_CHARGING:
			if(first)
			{
				first = 0;
				checkCounter = 60;	//charge for 60 seconds
			}
			if(Button::isAnyPressed())	//interrupt charging if button pressed
			{
				wakeReason = WAKEREASON_BUTTON;	//wake reason Button Press
				switchTo(STATE_WAKEUP);
				break;
			}
			if(checkCounter%2)	//every 2 seconds, pulse load and blink
			{
				Power::setLoad(true);
				Led::Blink(LED_GREEN,2,50);
				Power::setLoad(false);
			}
			if(checkCounter)
			{
				checkCounter--;
				Timer::shortSleep(1000);
			}
			else
			{
				switchTo(STATE_SLEEP);	//done charging
			}
			break;
		case STATE_WAKEUP:
			if(first)
			{
				first = 0;
				checkCounter = 3;	//check every 250ms 3 times
				tryCounter = 3;	//try every 10s 3 times
				wakeupStage = WAKESTAGE_FIRSTCHECK;
				Led::On(LED_GREEN);
				Power::setInputPower(1);
			}
			if(Power::isPowerConnected())
			{
				Led::Off(LED_GREEN);
				uint8_t vol_low = Power::isDeepDischarged();
				//successfully woken up
				if(vol_low)
				{
					Button::DeInit();	//reInit touch chip to prevent wrong button presses
					Timer::shortSleep(200);
				}
				switch (wakeReason)
				{
				case WAKEREASON_COUNTDOWN:
					Display::Wake();
					Display::StartAnimation(Display::ANIMATION_WAKE);
					Button::Init();
					while(!(Display::IsAnimationDone()) || Power::isCapLow())	//while animation not done or Cap Low
					{
						Display::Draw();
						Timer::shortSleep(30 + vol_low*60);	//Decrease Speed if Voltage Low
					}
					switchTo(STATE_PUMPING);		//switch to Pump State
					break;
				case WAKEREASON_CHARGING:
					Button::Init();
					switchTo(STATE_CHARGING);		//switch to Charge State
					break;
				case WAKEREASON_BUTTON:
					Display::Wake();
					Display::StartAnimation(Display::ANIMATION_WAKE);
					Button::Init();
					while(!(Display::IsAnimationDone()) || Power::isCapLow())	//while animation not done or Cap Low
					{
						Display::Draw();
						Timer::shortSleep(30 + vol_low*60);	//Decrease Speed if Voltage Low
					}
					Led::On(LED_BTN);			//turn on Button LED
					switchTo(STATE_SHOW_ERROR);		//switch to Error State -> Display State
					break;
				default:
					break;
				}
			}
			else
			{
				switch (wakeupStage)
				{
				case WAKESTAGE_FIRSTCHECK:
					if(checkCounter)
					{
						Power::setLoad(1);
						Timer::shortSleep(200);
						Power::setLoad(0);
						Timer::shortSleep(100);
						checkCounter--;
					}
					else
					{
						wakeupStage = WAKESTAGE_SECONDWAKE;
						Power::setInputPower(0);
						Led::Off(LED_GREEN);
						Timer::shortSleep(800);
						Led::On(LED_GREEN);
						Power::setInputPower(1);
						Timer::shortSleep(200);
					}
					break;
				case WAKESTAGE_SECONDWAKE:
					if(tryCounter)
					{
						tryCounter--;
						Power::setInputPower(0);
						Led::Off(LED_GREEN);
						Timer::shortSleep(10000);
						Led::On(LED_GREEN);
						Power::setInputPower(1);
						Power::setLoad(1);
						Timer::shortSleep(200);
						Power::setLoad(0);
						Timer::shortSleep(100);
					}
					else
					{
						Led::Off(LED_GREEN);
						Power::setInputPower(0);
						Data::SetError(Data::STATUS_PB_ERR);	//save error
						if(!Power::isCapLow()){	//if enough power, save flag
							Data::SaveError();
						}
						if(wakeReason == WAKEREASON_COUNTDOWN) 
							Data::setCustomCountdown(300);	//if wakeReason was Countdown, try 5 minutes later, as it is urgent
						switchTo(STATE_SLEEP);	//back to sleep
					}
					break;
				}
			}
			break;
		case STATE_PUMPING:
			/** Pump State
			 * Enable Pump and start Countdown
			 * if Countdown reached -> Display State
			 * if Man long Press -> stop Pump, Display State
			 * if Plus/Minus Short/Long Press -> Increment/Decrement Countdown
			 */
			if(first)
			{
				first = 0;
				Timer::shortSleep(500);	//wait for Cap to charge a bit
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
				Data::SetError(Data::STATUS_PB_ERR);
				if(!Power::isCapLow())
				{
					Data::SaveError();
				}
				switchTo(STATE_SLEEP);
				break;
			}
			break;

		case STATE_SHOW_ERROR:
			if(first)
			{
				first=0;
				if(Data::GetErrors() & _BV(Data::STATUS_PB_ERR))
				{
					status = Data::STATUS_PB_ERR;
				}
				else if (Data::GetErrors() & _BV(Data::STATUS_P1_ERR))
				{
					status = Data::STATUS_P1_ERR;
				}
				else if (Data::GetErrors() & _BV(Data::STATUS_P2_ERR))
				{
					status = Data::STATUS_P2_ERR;
				}
				else if (Data::GetErrors() & _BV(Data::STATUS_P3_ERR))
				{
					status = Data::STATUS_P3_ERR;
				}
				else if (Data::GetErrors() & _BV(Data::STATUS_EP_ERR))
				{
					status = Data::STATUS_EP_ERR;
				}
				else
				{	//no error
					switchTo(STATE_DISPLAY);
					break;
				}
				Display::ShowError(status);
			}
			if(Button::isPressed(Button::BUTTON_MAN) == Button::BUTTON_LONG_PRESS)	//ignore error forever
			{
				Data::ClearError(status);
				Data::SetIgnoreError(status);
				//write IGNORE
				Display::SetByte(0,Display::numToByte(1)); //I
				Display::SetByte(1,0x7D); //G
				Display::SetByte(2,0x54); //n
				Display::SetByte(3,0x5C); //o
				Display::SetByte(4,0x50); //r
				Display::SetByte(5,0x79); //E
				fade();
				switchTo(STATE_SHOW_ERROR);		//switch to Error for next error
				break;
			}
			//if any other button pressed
			if(Button::isPressed(Button::BUTTON_PLUS) == Button::BUTTON_SHORT_PRESS || Button::isPressed(Button::BUTTON_SET) == Button::BUTTON_SHORT_PRESS || Button::isPressed(Button::BUTTON_MINUS) == Button::BUTTON_SHORT_PRESS)
			{
				Data::ClearError(status);
				switchTo(STATE_SHOW_ERROR);
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
			case 0: //Build Date
				//Build Date
				Display::SetByte(0,Display::numToByte(BUILD_DAY/10));
				Display::SetByte(1,Display::numToByte(BUILD_DAY%10) | DEC_DOT);
				Display::SetByte(2,Display::numToByte(BUILD_MONTH/10));
				Display::SetByte(3,Display::numToByte(BUILD_MONTH%10) | DEC_DOT);
				Display::SetByte(4,Display::numToByte(BUILD_YEAR/10));
				Display::SetByte(5,Display::numToByte(BUILD_YEAR%10));
				break;
			case 1:
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
			case 2: //Current Runtime in 4 digit hour and 1 decimal point hour
				Display::Set4DigValue(0,currentRuntime/360);
				Display::SetByte(5,0x74); //h
				break;
			case 3: //Total Runtime
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