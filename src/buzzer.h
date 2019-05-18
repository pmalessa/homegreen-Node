/*
 * buzzer.h
 *
 *  Created on: 28.01.2019
 *      Author: pmale
 */

#ifndef BUZZER_H_
#define BUZZER_H_

#define BUZZER_TEMPO 120 //120 quarter per min -> 2 per second
#define EIGTH_MS 250

#define TONE_BOOT 1

void buzzer_init();
void buzzer_playTone(uint8_t tone_id);
void buzzer_SyncTask();

#endif /* BUZZER_H_ */
