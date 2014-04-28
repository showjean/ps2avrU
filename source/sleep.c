#ifndef SLEEP_C
#define SLEEP_C

#include <util/delay.h>

#include "sleep.h"
#include "ledrender.h"

static uint16_t _sleepCount = 0;
static uint8_t _sleepCount2 = 0;
static uint8_t _isSleep = 0;

void countSleep(void){
	if(_isSleep) return; 
	
	_sleepCount = _sleepCount + 1;

	if(_sleepCount >= SLEEP_COUNT_MAX){
		_sleepCount = 0;
		_sleepCount2 = _sleepCount2 + 1;
		if(_sleepCount2 == 10){
		    sleep();
		}
	}
}

void wakeUp(void){
	if(_isSleep){
		_isSleep = 0;

		wakeUpLED();
	}
	_sleepCount = 0;
	_sleepCount2 = 0;
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
