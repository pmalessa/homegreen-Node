/*
 * display.h
 *
 *  Created on: 29.01.2019
 *      Author: pmale
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

void display_init();
void display_clear();
void display_declear();
void display_SyncTask(); //1ms
void display_setValue(digit_t digit, uint16_t val); //val = (0.1 .. 1.0,1.1 .. 10.0,11.0 .. 99.0) * 10 = 1..990
uint8_t display_fadeUp(uint8_t val); //call till return==1
uint8_t display_fadeDown(uint8_t val); //call till return==1
void display_setbrightness(uint8_t val); //0..7
void display_setblinking(digit_t digit);
void display_clearBlinking();
void display_pumpanimation();
void display_resettimeout();
uint8_t display_gettimeout();
void display_setByte(uint8_t pos, uint8_t byte);
#endif /* DISPLAY_H_ */
