
#include "PLATFORM.h"
#include "display.h"
#include "timer.h"
#include "pump.h"
#include "power.h"
#include "data.h"
#include "temp.h"
#include "button.h"
#include "buzzer.h"

typedef enum{
	STATE_BOOT,
	STATE_DISPLAY,
	STATE_CONFIG,
	STATE_SLEEP,
	STATE_PUMPING,
	STATE_MAN_PUMPING,
	STATE_PB_EMPTY,
	STATE_BAT_LOW
}state_t;
state_t state = STATE_BOOT;
uint8_t first = 1;

volatile uint8_t wdt_interrupt = 0;

ISR(WDT_vect) {
	wdt_interrupt = 1;
	data_decCountdown(4);
}

void switchTo(state_t newstate)
{
	first = 1;
	state = newstate;
	button_clear();
}

void state_machine();

void anypress_callback()	//called if any Button pressed or released
{
	display_resettimeout();
	buzzer_playTone(TONE_BUT_CLICK);//buzzer sound
}

void powerchange_callback()	//called if power source changed BAT->PB or PB->BAT mööp 1 otterdamoo <3
{
	/*
	if(power_isPowerConnected())	//BAT -> PB
	{
		buzzer_playTone(TONE_POW_UP);
		display_init();
		switchTo(STATE_DISPLAY);
	}
	else	//PB -> BAT
	{
		buzzer_playTone(TONE_POW_DOWN);
		pump_disable();
		switchTo(STATE_SLEEP);
	}
	*/
}

int main (void) {
	//watchdog init
	cli();
	uint8_t reset_flag = MCUSR;
	MCUSR = 0;
	MCUSR &= ~(1<<WDRF);								//unlock step 1
	WDTCSR = (1 << WDCE) | (1 << WDE);					//unlock step 2
	WDTCSR = (1 << WDIE) | (1 << WDP3); 				//Set to Interrupt Mode and "every 4 s"

	timer_init();
	button_init();
	button_setPCCallback(&anypress_callback);
	pump_init();
	power_init();
	power_setCallback(&powerchange_callback);
	data_init();
//	temp_init();
	buzzer_init();

	sei();

	while(1)
	{
		state_machine();
	}
}

void fade()
{
	while(!display_fadeDown(0))
	{
		_delay_ms(40);
	}
	_delay_ms(400);
	while(!display_fadeUp(7))
	{
		_delay_ms(40);
	}
	_delay_ms(400);
	button_clear();
}

void state_machine()
{
	static digit_t curdigit = DIGIT_INTERVAL;
	static uint32_t prev_countdown = 0;
	static button_press press;

	switch (state) {
		case STATE_BOOT:
			if(first)
			{
				first = 0;
				power_setInputPower(1);
				if(power_isPowerConnected() == true)	//if PB connected
				{
					display_init();
					display_boot();
					buzzer_playTone(TONE_BOOT);
					switchTo(STATE_DISPLAY);
					break;
				}
				else									//if on BatPower
				{
					buzzer_playTone(TONE_BOOT2);
					switchTo(STATE_SLEEP);
					break;
				}
			}
			break;
		case STATE_DISPLAY:
			if(first)
			{
				first = 0;
				display_resettimeout();
				display_setValue(DIGIT_DURATION,data_get(DATA_DURATION));
				display_setValue(DIGIT_INTERVAL,data_get(DATA_INTERVAL));
				display_setValue(DIGIT_COUNTDOWN,data_getCountdownDisplay());
			}
			if(display_gettimeout() == 0)	//Display Timeout reached
			{
				fade();
				switchTo(STATE_SLEEP);
				break;
			}
			if(data_getCountdown() != prev_countdown)	//update Display
			{
				prev_countdown = data_getCountdown();
				display_setValue(DIGIT_COUNTDOWN,data_getCountdownDisplay());
				if(data_getCountdown() == 0)			//if countdown reached, switch to PUMPING
				{
					fade();
					data_resetCountdown();									//reset Countdown
					data_set(DATA_PUMP_DURATION,data_get(DATA_DURATION));	//set Pump Duration to configured duration
					switchTo(STATE_PUMPING);
					break;
				}
			}
			if((button_isPressed(BUTTON_MAN) == BUTTON_LONG_PRESSING))	//switch to MAN_PUMPING
			{
				display_clear();
				display_setByte(4,0x3F);	//O
				display_setByte(5,0x54);	//N
				fade();
				data_set(DATA_PUMP_DURATION,DATA_PUMP_DURATION_DEFAULT);	//2.0min
				switchTo(STATE_MAN_PUMPING);								//switch to MAN_PUMPING
				break;
			}
			if((button_isPressed(BUTTON_SET) == BUTTON_LONG_PRESSING))
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
				display_setValue(DIGIT_DURATION,data_get(DATA_DURATION));
				display_setValue(DIGIT_INTERVAL,data_get(DATA_INTERVAL));
				display_setValue(DIGIT_COUNTDOWN,data_getCountdownDisplay());
				display_setblinking(DIGIT_INTERVAL);
				curdigit = DIGIT_INTERVAL;
			}
			if(data_getCountdown() != prev_countdown)						//update Countdown
			{
				prev_countdown = data_getCountdown();
				display_setValue(DIGIT_COUNTDOWN,data_getCountdownDisplay());
			}
			press = button_isPressed(BUTTON_PLUS);							//get Button Plus Press
			if(press == BUTTON_LONG_PRESSING)								//if long Press
			{
				while(button_isPressed(BUTTON_PLUS))						//while pressed, fast increment
				{
					data_increment(curdigit);
					display_setValue(DIGIT_DURATION,data_get(DATA_DURATION));	//todo: only curdigit
					display_setValue(DIGIT_INTERVAL,data_get(DATA_INTERVAL));
					_delay_ms(100);
				}
			}
			else if(press == BUTTON_SHORT_PRESS)							//if short press, increment one step
			{
				data_increment(curdigit);
				display_setValue(DIGIT_DURATION,data_get(DATA_DURATION));		//todo: only curdigit
				display_setValue(DIGIT_INTERVAL,data_get(DATA_INTERVAL));
			}
			press = button_isPressed(BUTTON_MINUS);
			if(press == BUTTON_LONG_PRESSING)								//if long Press
			{
				while(button_isPressed(BUTTON_MINUS))						//while pressed, fast decrement
				{
					data_decrement(curdigit);
					display_setValue(DIGIT_DURATION,data_get(DATA_DURATION));	//todo: only curdigit
					display_setValue(DIGIT_INTERVAL,data_get(DATA_INTERVAL));
					_delay_ms(100);
				}
			}
			else if(press == BUTTON_SHORT_PRESS)							//if short press, decrement one step
			{
				data_decrement(curdigit);
				display_setValue(DIGIT_DURATION,data_get(DATA_DURATION));	//todo: only curdigit
				display_setValue(DIGIT_INTERVAL,data_get(DATA_INTERVAL));
			}
			press = button_isPressed(BUTTON_SET);							//get Set Button Press
			if(press == BUTTON_LONG_PRESSING || display_gettimeout() == 0)	//Long Press or IDLE timeout, Config done
			{
				data_save();												//save to EEPROM
				display_clearBlinking();
				fade();
				data_resetCountdown();										//reset Countdown
				switchTo(STATE_DISPLAY);									//switch to State Display
				break;
			}
			else if(press == BUTTON_SHORT_PRESS)							//Short SET Press, switch selected Digit
			{
				if(curdigit == DIGIT_DURATION)
				{
					curdigit = DIGIT_INTERVAL;
					display_setblinking(curdigit);
				}
				else
				{
					curdigit = DIGIT_DURATION;
					display_setblinking(curdigit);
				}
			}
			break;
		case STATE_SLEEP:
			if(first)
			{
				first = 0;
				//display_clear();					//deinit display
				buzzer_playTone(TONE_POW_DOWN);
				power_setInputPower(0);				//disable Powerbank
			}
		    set_sleep_mode(SLEEP_MODE_PWR_DOWN);	//Sleep mode: only wdt and pin interrupt
		    cli();									//disable interrupts
			sleep_enable();							//enable sleep
			//sleep_bod_disable();					//disable BOD for power save
			sei();									//enable interrupts
			sleep_cpu();							//sleep...
			/*zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz*/
			//waked up
			sleep_disable();						//disable sleep
		    sei();									//enable interrupts

			if(wdt_interrupt == 1)					//wdt interrupt wakeup
			{
				wdt_interrupt = 0;
				buzzer_playTone(TONE_CLICK);
				if(data_getCountdown() == 0)		//if countdown reached
				{
					power_setInputPower(1);			//enable Powerbank
					_delay_ms(500);					//wait for Powerbank to turn on
					if(power_isPowerConnected())	//check if Powerbank connected
					{
						display_init();
						buzzer_playTone(TONE_POW_UP);
						switchTo(STATE_DISPLAY);
						break;
					}
					else							//if not, switch to Powerbank Empty Error State
					{
						switchTo(STATE_PB_EMPTY);
						break;
					}
				}
			}
			else if(button_anyPressed())			//button interrupt wakeup
			{
				power_setInputPower(1);				//enable Powerbank
				_delay_ms(500);						//wait for Powerbank to turn on
				if(power_isPowerConnected())		//check if Powerbank connected
				{
					display_init();
					buzzer_playTone(TONE_POW_UP);
					switchTo(STATE_DISPLAY);		//switch to Display State
					break;
				}
				else								//if not, stay in sleep
				{
					//stay in sleep
				}
			}
			break;
		case STATE_PUMPING:
		case STATE_MAN_PUMPING:
			if(first)
			{
				first = 0;
				pump_enable(data_get(DATA_PUMP_DURATION));	//enable Pump for specified duration
			}
			if(pump_getCountdown() == 0)					//if pump duration reached, switch to Display State
			{
				data_resetCountdown();						//reset Countdown
				fade();
				switchTo(STATE_DISPLAY);
				break;
			}
			display_resettimeout(); 						//Display always on
			display_pumpanimation(pump_getCountdown());		//Display pump animation
			_delay_ms(100);
			press = button_isPressed(BUTTON_MAN);
			if(press == BUTTON_LONG_PRESSING)				//if Button MAN long pressed, disable Pump
			{
				display_clear();
				display_setByte(3,0x3F);	//O
				display_setByte(4,0x71);	//F
				display_setByte(5,0x71);	//F
                pump_disable();
				_delay_ms(1500);
				switchTo(STATE_DISPLAY);					//switch to Display State
				break;
			}

			press = button_isPressed(BUTTON_PLUS);
			if(press == BUTTON_LONG_PRESSING)				//if Plus Button long pressed, fast increment
			{
				while(button_isPressed(BUTTON_PLUS))
				{
					data_increment(DATA_PUMP_DURATION);
					pump_enable(data_get(DATA_PUMP_DURATION));
					display_setValue(DIGIT_COUNTDOWN,data_get(DATA_PUMP_DURATION));
					_delay_ms(100);
				}
			}
			else if(press == BUTTON_SHORT_PRESS)			//if short Press, one increment
			{
				data_increment(DATA_PUMP_DURATION);
				pump_enable(data_get(DATA_PUMP_DURATION));
				display_setValue(DIGIT_COUNTDOWN,data_get(DATA_PUMP_DURATION));
			}

			press = button_isPressed(BUTTON_MINUS);
			if(press == BUTTON_LONG_PRESSING)				//if Minus Button long pressed, fast decrement
			{
				while(button_isPressed(BUTTON_MINUS))
				{
					data_decrement(DATA_PUMP_DURATION);
					pump_enable(data_get(DATA_PUMP_DURATION));
					display_setValue(DIGIT_COUNTDOWN,data_get(DATA_PUMP_DURATION));
					_delay_ms(100);
				}
			}
			else if(press == BUTTON_SHORT_PRESS)			//if short pressed, one decrement
			{
				data_decrement(DATA_PUMP_DURATION);
				pump_enable(data_get(DATA_PUMP_DURATION));
				display_setValue(DIGIT_COUNTDOWN,data_get(DATA_PUMP_DURATION));
			}
			break;
		case STATE_PB_EMPTY:
			//play Buzzer alarm till PB available
			//not yet implemented...
			display_init();
			switchTo(STATE_DISPLAY);		//switch to Display State
			break;
		case STATE_BAT_LOW:
			//play Buzzer alarm till new bat
			break;
	}
}
