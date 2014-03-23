#ifndef SLEEP_H
#define SLEEP_H

#include "keymain.h"

#ifdef DEBUG_SLEEP_H
	#define SLEEP_COUNT_MAX 30000
#else
	#define SLEEP_COUNT_MAX 300000
#endif

void countSleep(void);

void wakeUp(void);

void sleep(void);

uint8_t isSleep(void);

#endif