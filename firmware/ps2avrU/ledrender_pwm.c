#include "timerinclude.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/eeprom.h>

#include <util/delay.h>
#include <string.h>

// #include "print.h"
#include "ledrender.h"

void setPWM(int xValue){
	timer1PWMBSet(xValue);
}


void initFullLEDState(void) {

	if(ledInited) return;
	ledInited = 1;

	/*
		timer0 = usb, ps/2 interface
		timer1 = pwm
		timer2 = custom macro
	*/

	timer1Init();
		
	timer1PWMInit(8);
	timer1PWMBOn();

//	DEBUG_PRINT(("______________________ init LED State_____________\n"));

	initFullLEDStateAfter();

}

