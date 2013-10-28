#ifndef SLEEP_C
#define SLEEP_C

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/eeprom.h>

#include <util/delay.h>
#include <string.h>

#include "sleep.h"
#include "ledrender.h"

static long int _sleepCount = 0;
static uint8_t _isSleep = 0;

void countSleep(void){
	if(_isSleep) return; 
	
	++_sleepCount;

	if(_sleepCount >= SLEEP_COUNT_MAX){
		_sleepCount = 0;
		sleep();
	}
}

void wakeUp(void){
	if(_isSleep){
		_isSleep = 0;

		wakeUpLED();
	}
	_sleepCount = 0;
}

void sleep(void){
	if(_isSleep == 0){
		_isSleep = 1;

		sleepLED();
	}
}

uint8_t isSleep(void){
	return _isSleep;
}

#endif
