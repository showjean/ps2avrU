#ifndef LEDRENDER_INC_C
#define LEDRENDER_INC_C

#include <avr/io.h>
#include <stdio.h>
#include "fncontrol.h"
#include "ledrender.h"
#include "hardwareinfo.h"
#include "ledconfig.h"
#include "bootmapper.h"

static uint8_t LEDstate = 0;     ///< current state of the LEDs
#define blinkLedCountDelay 900

static uint8_t ledBlinkCount = 0;
static uint8_t targetLED;
static uint8_t targetStatus;
/*
 * xPrev : LED의 이전 상태를 저장하고 있다. 이정 상태와 다를 경우에만 깜박이도록.
 */
void getLedBlink(uint8_t xLed, bool xOnOffStatus, uint8_t *xPrevState, uint8_t *xCount){
    if ((LEDstate & xLed) && !(*xPrevState & xLed)) { // light up

//		*xPrevState = LEDstate;
    	if(!xOnOffStatus){
    	    *xCount =  5;	//on off on off 1
		}else{
		    *xCount =  4;	// off on off 1
		}

	} else if(!(LEDstate & xLed) && (*xPrevState & xLed)){

//		*xPrevState = LEDstate;
		if(!xOnOffStatus){
		    *xCount =  3;	// on off 1
		}else{
		    *xCount =  2;	// off 1
		}

	}

    if(*xCount > 0)
    {
        targetStatus = xOnOffStatus;

        if(xLed == LED_STATE_NUM)
        {
            targetLED = LEDNUM;
        }
        else if(xLed == LED_STATE_CAPS)
        {
            targetLED = LEDCAPS;
        }
        else if(xLed == LED_STATE_SCROLL)
        {
            targetLED = LEDSCROLL;
        }
    }
}

void blinkIndicateLED(void) {
	static int counter = 0;
	const int countMAX = 100;
	//on off on off
	if(ledBlinkCount > 0){
		counter++;
		if(counter > countMAX){
			if(ledBlinkCount == 5 || ledBlinkCount == 3){
				turnOnLED(targetLED);
			}else if(ledBlinkCount == 4 || ledBlinkCount == 2){
				turnOffLED(targetLED);
			}else{
                if(targetStatus){
                    turnOnLED(targetLED);
                }
			}
			counter = 0;

			ledBlinkCount--;
		}
	}else{
		counter = 0;
	}
}


void blinkBootMapperLED(void) {
#ifdef ENABLE_BOOTMAPPER
	static int gDelayCounter = 0;
	static uint8_t gLEDState = 1;

	if(isBootMapper()){
		++gDelayCounter;
		if(gDelayCounter > blinkLedCountDelay){
            if(gLEDState == 1){
                turnOffLED(LEDCAPS);
            }else{
                turnOnLED(LEDCAPS);
            }
            gDelayCounter = 0;
            gLEDState ^= 1;
		}
	}else{
		gLEDState = 1;
	}
#endif
}

#define IS_LIGHT_UP_NL  LEDstate & LED_STATE_NUM
#define IS_LIGHT_UP_CL  LEDstate & LED_STATE_CAPS
#define IS_LIGHT_UP_SL  LEDstate & LED_STATE_SCROLL

void blinkOnce(const int xStayMs){
	if(getBeyondFnLed() == BEYOND_FN_LED_NL){
		if (getBeyondFN()) { // light up num lock on FN2 toggle
	        turnOffLED(LEDNUM);//PORTLEDS |= (1 << LEDNUM);	//
	    } else {
	        turnOnLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);	//
	    }
	}else{
		if ( IS_LIGHT_UP_NL ) { // light up num lock
	        turnOffLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);	//
	    }else{
	        turnOnLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);	//
	    }
	}
    if ( IS_LIGHT_UP_CL ) { // light up caps lock
        turnOffLED(LEDCAPS); //PORTLEDS &= ~(1 << LEDCAPS);	//
    } else {
        turnOnLED(LEDCAPS); //PORTLEDS |= (1 << LEDCAPS);	//
    }
	/*
		_delay_ms()에 xStayMs를 인자로 넣으면 hex 파일의 용량이 0x1000가량 증가한다.
		매뉴얼 펑션으로 _delay_ms(1)을 ms 만큼 루프시키도록 만들어서 사용;
	*/
	__delay_ms(xStayMs);

	if(getBeyondFnLed() == BEYOND_FN_LED_NL){
		if (getBeyondFN()) { // light up num lock on FN2 toggle
	        turnOnLED(LEDNUM);//PORTLEDS |= (1 << LEDNUM);	//
	    } else {
	        turnOffLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);	//
	    }
	}else{
		if ( IS_LIGHT_UP_NL ) { // light up num lock
	        turnOnLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);	//
	    }else{
	        turnOffLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);	//
	    }
	}
    if ( IS_LIGHT_UP_CL ) { // light up caps lock
        turnOnLED(LEDCAPS); //PORTLEDS |= (1 << LEDCAPS);	//
    } else {
        turnOffLED(LEDCAPS); //PORTLEDS &= ~(1 << LEDCAPS);	//
    }
}

void setLed(uint8_t xLed, bool xBool) {
	if (xBool) {
		if (xLed == LED_STATE_NUM) {
			turnOnLED(LEDNUM);
		} else if (xLed == LED_STATE_CAPS) {
			turnOnLED(LEDCAPS);
		}
#ifdef LEDSCROLL
		else if(xLed == LED_STATE_SCROLL) {
			turnOnLED(LEDSCROLL);
		}
#endif
	} else {
		if (xLed == LED_STATE_NUM) {
			turnOffLED(LEDNUM);
		} else if (xLed == LED_STATE_CAPS) {
			turnOffLED(LEDCAPS);
		}
#ifdef LEDSCROLL
		else if(xLed == LED_STATE_SCROLL) {
			turnOffLED(LEDSCROLL);
		}
#endif
	}
}
#endif
