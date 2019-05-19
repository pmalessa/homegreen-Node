
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
	STATE_MAN_PUMPING
}state_t;
state_t state = STATE_BOOT;
uint8_t first = 1;

volatile uint8_t wdt_interrupt = 0;

ISR(WDT_vect) {
	wdt_interrupt = 1;
	data_decCountdown(1);	//SET NORMAL
}

void state_machine();

void anypress_callback()	//called by Pin Change Interrupt
{
	display_resettimeout();
	//buzzer sound
}

void powerchange_callback()
{
	if(power_isPowerConnected())
	{
		display_init();//change to external power
		switch (state) {
			case STATE_DISPLAY:
			case STATE_CONFIG:
			case STATE_SLEEP:
				state = STATE_DISPLAY;
				first = 1;	//reinit state
				break;
			case STATE_PUMPING:
			case STATE_MAN_PUMPING:
				pump_disable();
				state = STATE_DISPLAY;
				first = 1;	//reinit state
				break;
			default:
				break;
		}
	}
	else
	{
		display_clear();	//change to battery power
	}
}

int main (void) {
	//watchdog init
	cli();
	uint8_t reset_flag = MCUSR;
	MCUSR = 0;
	MCUSR &= ~(1<<WDRF);								//unlock step 1
	WDTCSR = (1 << WDCE) | (1 << WDE);					//unlock step 2
	WDTCSR = (1 << WDIE) | (1 << WDP2) | (1 << WDP1); 	//Set to Interrupt Mode and "every 1 s"

	timer_init();
	button_init();
	button_setPCCallback(&anypress_callback);
	pump_init();
	power_init();
	power_setCallback(&powerchange_callback);
	data_init();
	display_init();
	temp_init();
	buzzer_init();

	//display_boot();
	display_clear();
	display_setByte(0,0); //F
	display_setByte(1,0); //L
	display_setByte(2,(reset_flag&_BV(3)) ? 1 : 0); //reset_flag bit 3
	display_setByte(3,(reset_flag&_BV(2)) ? 1 : 0); //reset_flag bit 2
	display_setByte(4,(reset_flag&_BV(1)) ? 1 : 0); //reset_flag bit 1
	display_setByte(5,(reset_flag&_BV(0)) ? 1 : 0); //reset_flag bit 0
	_delay_ms(1000);
	display_boot();
	//buzzer_playTone(TONE_BOOT);
	sei();

	while(1)
	{
		state_machine();
	}
}

void switchTo(state_t newstate)
{
	first = 1;
	state = newstate;
	button_clear();
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
				switchTo(STATE_DISPLAY);
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
			if(display_gettimeout() == 0)
			{
				fade();
				switchTo(STATE_SLEEP);
				break;
			}
			if(data_getCountdown() != prev_countdown)
			{
				prev_countdown = data_getCountdown();
				display_setValue(DIGIT_COUNTDOWN,data_getCountdownDisplay());
				if(data_getCountdown() == 0)	//switch to PUMPING
				{
					fade();
					data_resetCountdown();
					data_set(DATA_PUMP_DURATION,data_get(DATA_DURATION));	//set Pump Duration to configured Duration
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
				switchTo(STATE_MAN_PUMPING);
				break;
			}
			if((button_isPressed(BUTTON_SET) == BUTTON_LONG_PRESSING))	//switch to CONFIG
			{
				fade();
				switchTo(STATE_CONFIG);
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
			if(data_getCountdown() != prev_countdown)
			{
				prev_countdown = data_getCountdown();
				display_setValue(DIGIT_COUNTDOWN,data_getCountdownDisplay());
			}
			press = button_isPressed(BUTTON_PLUS);
			if(press == BUTTON_LONG_PRESSING)
			{
				while(button_isPressed(BUTTON_PLUS))
				{
					data_increment(curdigit);
					display_setValue(DIGIT_DURATION,data_get(DATA_DURATION));
					display_setValue(DIGIT_INTERVAL,data_get(DATA_INTERVAL));
					_delay_ms(100);
				}
			}
			else if(press == BUTTON_SHORT_PRESS)
			{
				data_increment(curdigit);
				display_setValue(DIGIT_DURATION,data_get(DATA_DURATION));
				display_setValue(DIGIT_INTERVAL,data_get(DATA_INTERVAL));
			}
			press = button_isPressed(BUTTON_MINUS);
			if(press == BUTTON_LONG_PRESSING)
			{
				while(button_isPressed(BUTTON_MINUS))
				{
					data_decrement(curdigit);
					display_setValue(DIGIT_DURATION,data_get(DATA_DURATION));
					display_setValue(DIGIT_INTERVAL,data_get(DATA_INTERVAL));
					_delay_ms(100);
				}
			}
			else if(press == BUTTON_SHORT_PRESS)
			{
				data_decrement(curdigit);
				display_setValue(DIGIT_DURATION,data_get(DATA_DURATION));
				display_setValue(DIGIT_INTERVAL,data_get(DATA_INTERVAL));
			}
			press = button_isPressed(BUTTON_SET);
			if(press == BUTTON_LONG_PRESSING || display_gettimeout() == 0)	//Config done or IDLE timeout
			{
				data_save();					//save to EEPROM
				display_clearBlinking();
				fade();
				data_resetCountdown();	//reset Countdown
				switchTo(STATE_DISPLAY);
				break;
			}
			else if(press == BUTTON_SHORT_PRESS)
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
				display_clear();//deinit display
				power_setInputPower(0);
			}
		    set_sleep_mode(SLEEP_MODE_PWR_DOWN);	//only wdt and pin interrupt
		    cli();									//disable interrupts
			sleep_enable();							//enable sleep
			//sleep_bod_disable();					//disable BOD for power save
			sei();									//enable interrupts
			sleep_cpu();							//sleep...
			sleep_disable();						//disable sleep
		    sei();									//enable interrupts
		    //waked up

			if(wdt_interrupt == 1)	//wdt interrupt wakeup
			{
				wdt_interrupt = 0;
				if(data_getCountdown() == 0)
				{
					power_setInputPower(1);
					display_init();
					switchTo(STATE_DISPLAY);
				}
			}
			else if(button_anyPressed())	//button interrupt wakeup
			{
				power_setInputPower(1);
				display_init();
				switchTo(STATE_DISPLAY);
			}

			break;
		case STATE_PUMPING:
		case STATE_MAN_PUMPING:
			if(first)
			{
				first = 0;
				pump_enable(data_get(DATA_PUMP_DURATION));	//convert to seconds
			}
			if(pump_getCountdown() == 0)
			{
				data_resetCountdown();	//reset Countdown
				fade();
				switchTo(STATE_DISPLAY);
			}
			display_resettimeout(); //no timeout
			display_pumpanimation(pump_getCountdown());
			_delay_ms(100);
			if(button_isPressed(BUTTON_MAN) == BUTTON_LONG_PRESSING)
			{
				display_resettimeout();
				display_clear();
				display_setByte(3,0x3F);	//O
				display_setByte(4,0x71);	//F
				display_setByte(5,0x71);	//F
                pump_disable();
				_delay_ms(1500);
				switchTo(STATE_DISPLAY);
			}

			press = button_isPressed(BUTTON_PLUS);
			if(press == BUTTON_LONG_PRESSING)
			{
				while(button_isPressed(BUTTON_PLUS))
				{
					data_increment(DATA_PUMP_DURATION);
					pump_enable(data_get(DATA_PUMP_DURATION));
					display_setValue(DIGIT_COUNTDOWN,data_get(DATA_PUMP_DURATION));
					_delay_ms(100);
				}
			}
			else if(press == BUTTON_SHORT_PRESS)
			{
				data_increment(DATA_PUMP_DURATION);
				pump_enable(data_get(DATA_PUMP_DURATION));
				display_setValue(DIGIT_COUNTDOWN,data_get(DATA_PUMP_DURATION));
			}

			press = button_isPressed(BUTTON_MINUS);
			if(press == BUTTON_LONG_PRESSING)
			{
				while(button_isPressed(BUTTON_MINUS))
				{
					data_decrement(DATA_PUMP_DURATION);
					pump_enable(data_get(DATA_PUMP_DURATION));
					display_setValue(DIGIT_COUNTDOWN,data_get(DATA_PUMP_DURATION));
					_delay_ms(100);
				}
			}
			else if(press == BUTTON_SHORT_PRESS)
			{
				data_decrement(DATA_PUMP_DURATION);
				pump_enable(data_get(DATA_PUMP_DURATION));
				display_setValue(DIGIT_COUNTDOWN,data_get(DATA_PUMP_DURATION));
			}
			break;
	}
}
