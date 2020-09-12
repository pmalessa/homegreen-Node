#include "Led.hpp"

void Led::Init()
{   
    LED_RED_DDR |= _BV(LED_RED_PIN);    //Red LED Output
    LED_GREEN_DDR |= _BV(LED_GREEN_PIN);    //Green LED Output
    LED_BTN_DDR |= _BV(LED_BTN_PIN);    //LED Button Output
    LED_RED_PORT &= ~_BV(LED_RED_PIN);    //LED low
    LED_GREEN_PORT &= ~_BV(LED_GREEN_PIN);    //LED low
    LED_BTN_PORT &= ~_BV(LED_BTN_PIN);    //LED low
}

void Led::Blink(uint8_t ledID, uint8_t nr, uint16_t delay)
{
    for(uint8_t i=0;i<nr;i++)
    {
        On(ledID);
        for (uint16_t j = 0; j < delay; j++)
        {
            Timer::shortSleep(1);
        }
        Off(ledID);
        for (uint16_t j = 0; j < delay; j++)
        {
            Timer::shortSleep(1);
        }
    }
}
void Led::On(uint8_t ledID)
{
    switch (ledID)
    {
    case LED_RED:
        LED_RED_PORT |= _BV(LED_RED_PIN);    //LED high
        break;
    case LED_GREEN:
        LED_GREEN_PORT |= _BV(LED_GREEN_PIN);    //LED high
        break;
    case LED_BTN:
        LED_BTN_PORT |= _BV(LED_BTN_PIN);    //LED high
        break;
    }
}
void Led::Off(uint8_t ledID)
{
    switch (ledID)
    {
    case LED_RED:
        LED_RED_PORT &= ~_BV(LED_RED_PIN);    //LED high
        break;
    case LED_GREEN:
        LED_GREEN_PORT &= ~_BV(LED_GREEN_PIN);    //LED high
        break;
    case LED_BTN:
        LED_BTN_PORT &= ~_BV(LED_BTN_PIN);    //LED high
        break;
    }
}