#ifndef LEDRENDER_INC_C
#define LEDRENDER_INC_C

#include <avr/io.h>
#include <stdio.h>
#include "ledrender.h"
#include "hardwareinfo.h"
#include "ledconfig.h"
#include "bootmapper.h"

#define blinkLedCountDelay 900

static uint8_t ledBlinkNumLockCount = 0;

void blinkNumLockLED(void) {
	static int counter = 0;
	const int countMAX = 100;
	//on off on off
	if(ledBlinkNumLockCount > 0){
		counter++;
		if(counter > countMAX){
			if(ledBlinkNumLockCount == 5 || ledBlinkNumLockCount == 3){
				turnOnLED(LEDNUM); //PORTLEDS |= (1 << LEDCAPS);
			}else if(ledBlinkNumLockCount == 4 || ledBlinkNumLockCount == 2){
				turnOffLED(LEDNUM); //PORTLEDS &= ~(1 << LEDCAPS);
			}else{
				if(isBeyondFnLedEnabled()){
					if(isBeyondFN()){
						turnOnLED(LEDNUM);
					}
				}else{
					if((getLEDState() & LED_STATE_NUM)){
						turnOnLED(LEDNUM);
					}
				}
			}
			counter = 0;

			ledBlinkNumLockCount--;
		}
	}else{
		counter = 0;
	}
}


void blinkCapsLockLED(void) {
//	static int gCounter = 0;
	static int gDelayCounter = 0;
	static uint8_t gLEDState = 1;
//	const int gCountMAX = blinkLedCountDelay; // 200;
	uint8_t gIsOn = 0;

#ifdef ENABLE_BOOTMAPPER
	if(isBootMapper()){
		gIsOn = 1;
	}
#endif

	if(gIsOn == 1){
		++gDelayCounter;
		if(gDelayCounter > blinkLedCountDelay){	//setDelay(blinkLedCountDelay)){
//			++gCounter;
//			if(gCounter > gCountMAX){
//				if(getLEDState() & LED_STATE_CAPS){	// Caps Lock이 켜져 있을때는 커졌다 켜지고;
					if(gLEDState == 1){
						turnOffLED(LEDCAPS);
					}else{
						turnOnLED(LEDCAPS);
//						gDelayCounter = 0;
					}
				/*}else{	// Caps Lock이 꺼져 있을 때는 켜졌다 꺼진다.
					if(gLEDState == 1){
						turnOnLED(LEDCAPS);
					}else{
						turnOffLED(LEDCAPS);
//						gDelayCounter = 0;
					}
				}*/
				gDelayCounter = 0;
				gLEDState ^= 1;
//				gCounter = 0;
//			}
		}
	}else{
//		gCounter = 0;
		gLEDState = 1;
	}
}

#endif
