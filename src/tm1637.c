//
// AVR TM1637 "Library" v1.01
// Enables control of TM1637 chip based modules, using direct port access.
//
// Copyright (c) 2015 IronCreek Software
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include "PLATFORM.h"
#include "tm1637.h"

#define TM_CLK_LOW()            (TM_OUT &= ~TM_BIT_CLK)
#define TM_CLK_HIGH()           (TM_OUT |= TM_BIT_CLK)
#define TM_DAT_LOW()            (TM_OUT &= ~TM_BIT_DAT)
#define TM_DAT_HIGH()           (TM_OUT |= TM_BIT_DAT)

#define TM_CLK_FLOAT()

// Instructions
#define TM_DATA_CMD             0x40
#define TM_DISP_CTRL            0x80
#define TM_ADDR_CMD             0xC0

// Data command set
#define TM_WRITE_DISP           0x00
#define TM_READ_KEYS            0x02
#define TM_FIXED_ADDR           0x04

// Display control command
#define TM_DISP_PWM_MASK        0x07 // First 3 bits are brightness (PWM controlled)
#define TM_DISP_ENABLE          0x08

#define DELAY_US                50

#define TM_DOT          0x80

#define TM_MINUS        0x40
#define TM_PLUS         0x44
#define TM_BLANK        0x00
#define TM_DEGREES      0x63
#define TM_UNDERSCORE   0x08
#define TM_EQUALS       0x48
#define TM_CHAR_ERR     0x49

volatile uint8_t dotmask = 0;

uint8_t TM1637_map_char(char ch)
{
    uint8_t rc = 0;

    switch (ch)
    {
        case '-': rc = TM_MINUS; break;
        case '+': rc = TM_PLUS; break;
        case ' ': rc = TM_BLANK; break;
        case '^': rc = TM_DEGREES; break;
        case '_': rc = TM_UNDERSCORE; break;
        case '=': rc = TM_EQUALS; break;
        default:
            break;
    }

    return rc;
}

//      Bits:                 Hex:
//        -- 0 --               -- 01 --
//       |       |             |        |
//       5       1            20        02
//       |       |             |        |
//        -- 6 --               -- 40 --
//       |       |             |        |
//       4       2            10        04
//       |       |             |        |
//        -- 3 --  .7           -- 08 --   .80


PROGMEM const uint8_t TM_DIGITS[] =
{
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9

    0x77, // A
    0x7C, // b
    0x39, // C
    0x5E, // d
    0x79, // E
    0x71, // F

    // HEX DIGITS END

    0x3D, // G
    0x76, // H
    0x06, // I
    0x1F, // J
    0x76, // K (same as H)
    0x38, // L
    0x15, // M
    0x54, // n
    0x3F, // O
    0x73, // P
    0x67, // Q
    0x50, // r
    0x6D, // S
    0x78, // t
    0x3E, // U
    0x1C, // V
    0x2A, // W
    0x76, // X (same as H)
    0x6E, // Y
    0x5B  // Z
};

void port_setup()
{
	TM_DDR |= TM_BIT_CLK | TM_BIT_DAT;
	TM_OUT |= TM_BIT_CLK;
}

void start()
{
	TM_CLK_HIGH();
	TM_DAT_HIGH();
	_delay_us(DELAY_US);

	TM_DAT_LOW();
	_delay_us(DELAY_US);
}

void stop()
{
	TM_CLK_LOW();
	_delay_us(DELAY_US);

	TM_CLK_HIGH();
	TM_DAT_LOW();
	_delay_us(DELAY_US);

	TM_DAT_HIGH();
}

void send(uint8_t b)
{
	// Clock data bits
	for (uint8_t i = 8; i; --i, b >>= 1)
	{
		TM_CLK_LOW();
		if (b & 1)
			TM_DAT_HIGH();
		else
			TM_DAT_LOW();
		_delay_us(DELAY_US);

		TM_CLK_HIGH();
		_delay_us(DELAY_US);
	}

	// Clock out ACK bit; not checking if it worked...
	TM_CLK_LOW();
	TM_DAT_LOW();
	_delay_us(DELAY_US);

	TM_CLK_HIGH();
	_delay_us(DELAY_US);
}

void send_cmd(uint8_t cmd)
{
	start();
	send(cmd);
	stop();
}

void send_data(uint8_t addr, uint8_t data)
{
	send_cmd(TM_DATA_CMD | TM_FIXED_ADDR);

	start();
	send(TM_ADDR_CMD | addr);
	send(data);
	stop();

	_delay_us(DELAY_US);
}

uint8_t offset_digits(uint32_t num)
{
	uint8_t digits = 0;
	while (num >= 10)
	{
		num /= 10;
		++digits;
	}
	return digits;
}

void tm1637_Init()
{
    port_setup();

    send_cmd(TM_DATA_CMD | TM_WRITE_DISP);
    send_cmd(TM_DISP_CTRL | TM_DISP_ENABLE | TM_DISP_PWM_MASK);

    tm1637_Clear();
}

void tm1637_Clear()
{
    for (uint8_t a = 0; a != TM1637_DIGITS; ++a)
        send_data(a, 0x00);
}

void tm1637_setByte(uint8_t position, uint8_t b)
{
    send_data(position, b | (dotmask & (1 << position) ? TM_DOT : 0));
}

void tm1637_setDigit(uint8_t position, uint8_t digit)
{
	tm1637_setByte(position, pgm_read_byte(&TM_DIGITS[digit & 0xF]));
}

void tm1637_setNumber(uint32_t number, uint8_t offset, uint8_t align)
{
    if (align == TM_LEFT)
        offset += offset_digits(number);

    while (number && offset != 0xFF)
    {
    	tm1637_setDigit(offset--, number % 10);
        number /= 10;
    }
}

void tm1637_setNumberPad(uint32_t number, uint8_t offset, uint8_t width, uint8_t pad)
{
    while (number && width-- && offset != 0xFF)
    {
    	tm1637_setDigit(offset--, number % 10);
        number /= 10;
    }

    while (width -- && offset != 0xFF)
    	tm1637_setByte(offset--, pad);
}

void tm1637_setNumberHex(uint32_t number, uint8_t offset, uint8_t width, uint8_t pad)
{
    while (number && width-- && offset != 0xFF)
    {
    	tm1637_setDigit(offset--, number & 0x0F);
        number >>= 4;
    }

    while (width -- && offset != 0xFF)
    	tm1637_setByte(offset--, pad);
}

void tm1637_setChar(uint8_t position, char ch)
{
    uint8_t b = TM1637_map_char(ch);
    if (b || ch == ' ')
    	tm1637_setByte(position, b);

    else if (ch >= 'a' && ch <= 'z')
    	tm1637_setByte(position, pgm_read_byte(&TM_DIGITS[ch - 'a' + 10]));

    else if (ch >= 'A' && ch <= 'Z')
    	tm1637_setByte(position, pgm_read_byte(&TM_DIGITS[ch - 'A' + 10]));

    else if (ch >= '0' && ch <= '9')
    	tm1637_setByte(position, pgm_read_byte(&TM_DIGITS[ch - '0']));
}

void tm1637_setChars(char* value, uint8_t offset)
{
    while (*value)
    	tm1637_setChar(offset++, *value++);
}

void tm1637_scrollChars(char* value)
{
    uint32_t offset = 0;
    if (value == 0 || *value == 0)
        return;

    while(1)
    {
        for (uint8_t i = 0; i != TM1637_DIGITS; ++i)
        {
            char *p = value + offset;
            tm1637_setChar(i, p[i]);

            if (!p[i+1])
            {
                _delay_ms(250);
                return;
            }
        }

        ++offset;
        _delay_ms(250);
    }
}

void tm1637_setDots(uint8_t mask)
{
    dotmask = mask;
}

void tm1637_setBrightness(uint8_t brightness)
{
    send_cmd(TM_DISP_CTRL | TM_DISP_ENABLE | (brightness & TM_DISP_PWM_MASK));
}

uint8_t tm1637_NrToByte(uint8_t nr)
{
	return pgm_read_byte(&TM_DIGITS[nr & 0xF]);
}
