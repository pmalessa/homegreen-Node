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

#ifndef TM1637_H_
#define TM1637_H_


#include <stdint.h>

// ----------------------------------------------------------------------------
// PIN / PORT configuration
// NOTE: All three pins used must be bits on the same PORT register (ex. PORTB)
// ----------------------------------------------------------------------------

#define TM_OUT          PORTD
#define TM_IN           PIND
#define TM_DDR          DDRD
#define TM_BIT_CLK      _BV(PD5)
#define TM_BIT_DAT      _BV(PD6)

// ----------------------------------------------------------------------------
// Functions and parameters
// ----------------------------------------------------------------------------

// Parameters for setNumber()
#define TM_RIGHT        0x01
#define TM_LEFT         0x00

// Parameters for setNumberPad()
#define TM_PAD_SPACE    0x00
#define TM_PAD_0        0x3F

#define TM1637_DIGITS   6

void tm1637_Init();
void tm1637_deInit();
void tm1637_Clear();	// Clear the 7-segment displays (only)
void tm1637_setByte(uint8_t position, uint8_t b); // Set a single 7-segment display to the given byte value.
void setDigit(uint8_t position, uint8_t digit); // Display a single digit at the given position.

// Display an unsigned number at a given offset and alignment.
void tm1637_setNumber(uint32_t number, uint8_t offset, uint8_t align);

// Display an unsigned number at a given offset and pad it with 0's or
// spaces to a desired with. This function is helpful when the numbers can
// fluctuate in length (ex. 100, 5, 1000) to avoid flickering segments.
void tm1637_setNumberPad(uint32_t number, uint8_t offset, uint8_t width, uint8_t pad);

// Display an unsigned number in hex format at a given offset and pad it
// with 0's or spaces to a desired with.
void tm1637_setNumberHex(uint32_t number, uint8_t offset, uint8_t width, uint8_t pad);

// Draw a character at a given position.
// Not all characters are supported, check TM1637Font.h for an overview.
void tm1637_setChar(uint8_t position, char ch);

// Display a string starting at a given offset.
void tm1637_setChars(char* value, uint8_t offset);

// Scroll characters (scrolls <--- left)
void tm1637_scrollChars(char* value);

// Set which "dots" should be enabled.
// Mask is mapped right to left (ex. 0x01 = right-most dot)
void tm1637_setDots(uint8_t mask);

// Set the brightness between 0 and 7
void tm1637_setBrightness(uint8_t brightness);

uint8_t tm1637_NrToByte(uint8_t nr);
void tm1637_setDigit(uint8_t position, uint8_t digit);
#endif
