#ifndef TIMERINCLUDE_H
#define TIMERINCLUDE_H

#ifdef __AVR_ATmega128__
	#include "timer128.h"
#else
	#include "timer.h"
#endif

#endif