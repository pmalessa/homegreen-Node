#include "Led.hpp"

void Led::Init()
{   
    LED_DDR |= _BV(LED_PIN);    //LED Output
    LED_PORT &= ~(_BV(LED_PIN));    //LED low
}
void Led::Blink(uint8_t nr, uint16_t delay)
{
    for(uint8_t i=0;i<nr;i++)
    {
        On();
        for (uint16_t j = 0; i < delay; j++)
        {
            _delay_ms(1);
        }
        Off();
        for (uint16_t j = 0; i < delay; j++)
        {
            _delay_ms(1);
        }
    }
}
void Led::On()
{
    LED_PORT |= (_BV(LED_PIN));    //LED high
}
void Led::Off()
{
    LED_PORT &= ~(_BV(LED_PIN));    //LED low
}