/*
 * buzzer.h
 *
 *  Created on: 28.01.2019
 *      Author: pmale
 */

#ifndef BUZZER_H_
#define BUZZER_H_

#define TONE_BOOT 1
#define TONE_BOOT2 2
#define TONE_POW_UP 3
#define TONE_POW_DOWN 4
#define TONE_HEARTBEAT 5
#define TONE_CLICK 6
#define TONE_ALARM 7

void buzzer_init();
uint8_t buzzer_isPlaying();
void buzzer_playTone(uint8_t tone_id);
void buzzer_SyncTask();

#endif /* BUZZER_H_ */
