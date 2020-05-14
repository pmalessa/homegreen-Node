#include "Led.hpp"

void Led::Init()
{   
    LED_DDR |= _BV(LED_PIN);    //LED Output
    LED_PORT &= ~(_BV(LED_PIN));    //LED low
}

void Led::fadeAnimation()
{
    uint8_t cycles = 10, tenmsPerCycle = 50;
    for (uint16_t cy_cnt = 0; cy_cnt < cycles; cy_cnt++)    //each cycle
    {
        for (uint16_t ms_cnt = 0; ms_cnt < tenmsPerCycle; ms_cnt++)    //50 * 10ms = 500ms
        {
            On();
            for(uint8_t i=0; i<ms_cnt;i++)  _delay_us(200);  //0..50 * 200us = 0..10ms
            Off();
            for(uint8_t i=0; i<(tenmsPerCycle-ms_cnt);i++) _delay_us(200); //50..0 * 200us = 10..0ms
        }
        for (uint16_t ms_cnt = tenmsPerCycle; ms_cnt > 0; ms_cnt--)    //50 * 10ms = 500ms
        {
            On();
            for(uint8_t i=0; i<ms_cnt;i++)  _delay_us(200);  //0..50 * 200us = 0..10ms
            Off();
            for(uint8_t i=0; i<(tenmsPerCycle-ms_cnt);i++) _delay_us(200); //50..0 * 200us = 10..0ms
        }
        _delay_ms(tenmsPerCycle*10);
    }
    
}

void Led::Blink(uint8_t nr, uint16_t delay)
{
    for(uint8_t i=0;i<nr;i++)
    {
        On();
        for (uint16_t j = 0; j < delay; j++)
        {
            Timer::shortSleep(1);
        }
        Off();
        for (uint16_t j = 0; j < delay; j++)
        {
            Timer::shortSleep(1);
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