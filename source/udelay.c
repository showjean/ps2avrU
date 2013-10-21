#include "global.h"
#include "udelay.h"

static short tmp;
void _delay_us_m(unsigned short time_us) {
	unsigned short delay_loops;
	register unsigned short i;

	delay_loops = (time_us+3)/5*CYCLES_PER_US; // +3 for rounding up (dirty) 

	// one loop takes 5 cpu cycles 
	for (i=0; i < delay_loops; i++) {tmp++;};
}