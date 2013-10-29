#ifndef SLEEP_H
#define SLEEP_H

#include "keymain.h"
#include "ledrender.h"

#ifdef RELEASE
#define SLEEP_COUNT_MAX 300000
#else
#define SLEEP_COUNT_MAX 30000
#endif

extern void countSleep(void);

extern void wakeUp(void);

extern void sleep(void);

extern uint8_t isSleep(void);

#endif