#ifndef SLEEP_H
#define SLEEP_H

#include "ledrender.h"

#ifdef RELEASE
#define SLEEP_COUNT_MAX 300000
#else
#define SLEEP_COUNT_MAX 30000
#endif

long int _sleepCount = 0;
uint8_t _isSleep = 0;

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