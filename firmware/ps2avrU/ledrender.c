
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
#include "fncontrol.h"
#include "sleep.h"
#include "keymapper.h"
#include "main.h"
#include "bootmapper.h"
#include "ps2avru_util.h"

#define PWM_MAX 0xFF
#define LED_MODE_NUM 5

static int downLevelStay = 0;
static uint8_t downLevel = 0;
static const uint8_t downLevelMax = 12;
static uint8_t downLevelLife = 0;

static uint8_t ledInited = 0;
static uint8_t LEDstate = 0;     ///< current state of the LEDs
#ifndef SCROLL_LOCK_LED_IS_APART 
static uint8_t ledBlinkScrollLockCount = 0;
#endif
static uint8_t ledBlinkNumLockCount = 0;

static uint8_t _fullLEDMode = 0;	// 
static uint8_t _fullLEDMode_saved = 0;	// 

static int pwmCounter = 0;
static int pwmDir = 0;
static const uint8_t speed = 8;
static uint8_t _isDidSleep = 0;

static int ledStateCount = 0;

static uint8_t _ledBrightnessMax = PWM_MAX; 
static uint8_t _ledBrightnessMax_saved = 0;	// 
static const uint8_t ledBrightnessMin = 30;
static const uint8_t ledBrightnessStep = 25;

static int blinkLedCountDelay = 1800;

static void fadeLED(void);
static void setFullLEDState(void);

void initLED(void){	
	// led pin
	DDRD |= (LEDNUM | LEDCAPS | LEDFULLLED);	// output;
	PORTD &= ~(LEDNUM | LEDCAPS | LEDFULLLED);	// low
 
#ifdef LEDSCROLL
   	DDRD |= (LEDSCROLL);	// output;
	PORTD &= ~(LEDSCROLL);	// low
#endif
}

void initFullLEDStateAfter(void){

	_ledBrightnessMax = eeprom_read_byte((uint8_t *)EEPROM_LED_BRIGHTNESS);
	_ledBrightnessMax_saved = _ledBrightnessMax;

	_fullLEDMode = eeprom_read_byte((uint8_t *)EEPROM_LED_MODE);    // 1바이트 11번지 읽기, 기본값 0xFF ( 255)
	if(_fullLEDMode >= LED_MODE_NUM || _fullLEDMode < 0){
		_fullLEDMode = 0;
	}
	_fullLEDMode_saved = _fullLEDMode;

	setFullLEDState();

	blinkLedCountDelay = setDelay(blinkLedCountDelay);
}

void blinkOnce(const int xStayMs){
	if(isBeyondFnLedEnabled()){
		if (isBeyondFN()) { // light up num lock on FN2 toggle
	        turnOffLED(LEDNUM);//PORTLEDS |= (1 << LEDNUM);	//
	    } else {
	        turnOnLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);	//
	    }
	}else{
		if (LEDstate & LED_STATE_NUM) { // light up num lock
	        turnOffLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);	//
	    }else{
	        turnOnLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);	//    	
	    }
	}
    if (LEDstate & LED_STATE_CAPS) { // light up caps lock
        turnOffLED(LEDCAPS); //PORTLEDS &= ~(1 << LEDCAPS);	//
    } else {
        turnOnLED(LEDCAPS); //PORTLEDS |= (1 << LEDCAPS);	// 
    }
	/*
		_delay_ms()에 xStayMs를 인자로 넣으면 hex 파일의 용량이 0x1000가량 증가한다. 
		매뉴얼 펑션으로 _delay_ms(1)을 ms 만큼 루프시키도록 만들어서 사용;
	*/
	__delay_ms(xStayMs);	
	
	if(isBeyondFnLedEnabled()){
		if (isBeyondFN()) { // light up num lock on FN2 toggle
	        turnOnLED(LEDNUM);//PORTLEDS |= (1 << LEDNUM);	//
	    } else {
	        turnOffLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);	//
	    }
	}else{
		if (LEDstate & LED_STATE_NUM) { // light up num lock
	        turnOnLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);	//   
	    }else{
	        turnOffLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);	// 	
	    }
	}
    if (LEDstate & LED_STATE_CAPS) { // light up caps lock
        turnOnLED(LEDCAPS); //PORTLEDS |= (1 << LEDCAPS);	// 
    } else {
        turnOffLED(LEDCAPS); //PORTLEDS &= ~(1 << LEDCAPS);	//
    }	
}

void setLEDState(uint8_t xState){
	LEDstate = xState;
}
uint8_t getLEDState(void){
	return LEDstate;
}

static void setFullLEDState(void) {
	/* _fullLEDMode
		0 = off 
		1 = fading 
		2 = on 
		3 = key down level up
		4 = key down level down
	*/
	if(_fullLEDMode == 0){
		setPWM(0);
	}else if(_fullLEDMode == 1){
		pwmCounter = 0;
		pwmDir = 0;
		// setPWM(0);
	}else if(_fullLEDMode == 2){
		setPWM(_ledBrightnessMax);
	}else if(_fullLEDMode == 3 || _fullLEDMode == 4){
		downLevel = 3;
		if(_fullLEDMode == 4) downLevel = 1; 	// 3으로 시작하면 효과가 별로 안 남.
		downLevelLife = PWM_MAX * downLevel / downLevelMax;
	}
}

void increaseLedBrightness(void){
	if(_ledBrightnessMax == PWM_MAX) return;

	int gLedBrightness = _ledBrightnessMax + ledBrightnessStep;
	if(gLedBrightness > PWM_MAX){
		_ledBrightnessMax = PWM_MAX;
	}else{
		_ledBrightnessMax = gLedBrightness;		
	}
	if(_fullLEDMode == 2){
		setFullLEDState();
	}
	ledStateCount = 1;
}
void reduceLedBrightness(void){
	if(_ledBrightnessMax == ledBrightnessMin) return;

	_ledBrightnessMax = _ledBrightnessMax - ledBrightnessStep;
	if(_ledBrightnessMax < ledBrightnessMin){
		_ledBrightnessMax = ledBrightnessMin;
	}
	if(_fullLEDMode == 2){
		setFullLEDState();
	}
	ledStateCount = 1;
}
void changeFullLedState(uint8_t xFullLedMode){
	if(isSleep()){
		return;
	}
	
	//LED 모드를 변경한다.
	_fullLEDMode = (_fullLEDMode+1)%LED_MODE_NUM;
	ledStateCount = 1;
	//DEBUG_PRINT(("_fullLEDMode %02x\n ", _fullLEDMode));
	setFullLEDState();
}

void clearLEDInited(void){
	ledInited = 0;
}

void setLed(uint8_t xLed, bool xBool){
	if(xBool){
		if(xLed == LED_STATE_NUM){
			turnOnLED(LEDNUM);
		}else if(xLed == LED_STATE_CAPS){
			turnOnLED(LEDCAPS);
		}
#ifdef LEDSCROLL
		else if(xLed == LED_STATE_SCROLL){
			turnOnLED(LEDSCROLL);
		}
#endif
	}else{
		if(xLed == LED_STATE_NUM){
			turnOffLED(LEDNUM);
		}else if(xLed == LED_STATE_CAPS){
			turnOffLED(LEDCAPS);
		}
#ifdef LEDSCROLL
		else if(xLed == LED_STATE_SCROLL){
			turnOffLED(LEDSCROLL);
		}
#endif
	}
}

static uint8_t getLedBlnik(uint8_t xLed, bool xStatus, bool *xPrev){
	// static uint8_t prevSCRLED = 0;
	uint8_t gVal;
    if ((LEDstate & xLed) && *xPrev == false) { // light up
    	
		*xPrev = true;
		// delay 함수를 이용하면 ps2연결시 노이즈가 발생하는지 "방향키 상"이 
		// 연속으로 눌리는 현상이있어, 실시간으로 작동되도록 수정;
    	if(!xStatus){
			gVal =  5;	//on off on off 1
		}else{
			gVal =  4;	// off on off 1
		}
		
	} else if(!(LEDstate & xLed) &&  *xPrev == true){
		
		*xPrev = false;	
		if(!xStatus){
			gVal =  3;	// on off 1
		}else{
			gVal =  2;	// off 1
		}
	
	}
	return gVal;
}

void setLEDIndicate(void) {
	static bool prevNum = false;

	if(isBeyondFnLedEnabled()){
		ledBlinkNumLockCount = getLedBlnik(LED_STATE_NUM, isBeyondFN(), &prevNum);
	}else{
		if (LEDstate & LED_STATE_NUM) { // light up num lock
	        turnOnLED(LEDNUM);//PORTLEDS |= (1 << LEDNUM);	//
	    } else {
	        turnOffLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);	//
	    }
	}

    if (LEDstate & LED_STATE_CAPS) { // light up caps lock
        turnOnLED(LEDCAPS); //PORTLEDS |= (1 << LEDCAPS);	// 
    } else {
        turnOffLED(LEDCAPS); //PORTLEDS &= ~(1 << LEDCAPS);	//
    }

#ifdef LEDSCROLL
    if (LEDstate & LED_STATE_SCROLL) { // light up scroll lock
        turnOnLED(LEDSCROLL); //PORTLEDS |= (1 << LEDCAPS);	// 
    } else {
        turnOffLED(LEDSCROLL); //PORTLEDS &= ~(1 << LEDCAPS);	//
    }
#endif

#ifndef SCROLL_LOCK_LED_IS_APART   
	static bool prevScroll = false; 
	ledBlinkScrollLockCount = getLedBlnik(LED_STATE_SCROLL, (LEDstate & LED_STATE_CAPS), &prevScroll);
#endif
	
}

void applyKeyDownForFullLED(uint8_t keyidx, uint8_t col, uint8_t row, uint8_t isDown){
	if(!isDown) return;
	
	if(_fullLEDMode == 3){
			// 키를 누르면 값을 증가 시킨다.
		downLevelStay = 500; //511;
		if(downLevel < downLevelMax){
			downLevel++;
			downLevelLife = PWM_MAX * downLevel / downLevelMax;
				//DEBUG_PRINT(("increase downLevel : %d, life : %d\n", downLevel, downLevelLife));
		}
	}else if(_fullLEDMode == 4){
			// 키를 누르면 값을 감소 시킨다.
		downLevelStay = 500; //511;
		if(downLevel > 0){
			downLevel--;
			downLevelLife = PWM_MAX * downLevel / downLevelMax;
				//DEBUG_PRINT(("increase downLevel : %d, life : %d\n", downLevel, downLevelLife));
		}
	}
}


static void writeLEDMode(void) {
	// LED 모드 저장, 너무많은 eeprom접근을 막기위해 일정 간격으로 저장한다.
	const int countMAX = 1000;
	if(ledStateCount > 0 && ++ledStateCount == countMAX){
		// DEBUG_PRINT(("writeLEDMode : mode %d, br %d \n", _fullLEDMode, _ledBrightnessMax));
		ledStateCount = 0;
		if(_fullLEDMode != _fullLEDMode_saved){
			_fullLEDMode_saved = _fullLEDMode;
			eeprom_write_byte((uint8_t *)EEPROM_LED_MODE, _fullLEDMode);  // 1바이트 11번지 쓰기
			// DEBUG_PRINT(("eeprom_write_byte : mode %d \n", _fullLEDMode));
		}

		// brightness
		if(_ledBrightnessMax_saved != _ledBrightnessMax){
			_ledBrightnessMax_saved = _ledBrightnessMax;
			eeprom_write_byte((uint8_t *)EEPROM_LED_BRIGHTNESS, _ledBrightnessMax);  // 1바이트 9번지 쓰기
			// DEBUG_PRINT(("eeprom_write_byte : _ledBrightnessMax %d \n", _ledBrightnessMax));
		}
	}
}

static int getBrightness(int xValue){
	int gVal = xValue * _ledBrightnessMax / PWM_MAX;
	//DEBUG_PRINT(("getBrightness : xValue : %d, val %d \n", xValue, gVal));
	return gVal;
}

static void fadePWM(void){
	fadeLED();
}

// 
static void fadeLED(void) {

	if(_fullLEDMode == 1) {	

		if(pwmDir==0)
			setPWM((uint8_t)(getBrightness(pwmCounter/speed)));
		else if(pwmDir==2)
			setPWM((uint8_t)(getBrightness(PWM_MAX-pwmCounter/speed)));
		else if(pwmDir==1 || pwmDir==3)
			pwmCounter++;

		// pwmDir 0~3 : idle
		if( pwmCounter>=PWM_MAX*speed) {
			pwmCounter=0;
			pwmDir = (pwmDir+1)%4;
		}
		pwmCounter++;
		
	}else if(_fullLEDMode == 3){
		
		// 일정시간 유지
		if(downLevelStay > 0){
			downLevelStay--;
		}else{
			// 시간이 흐르면 레벨을 감소 시킨다.
			if(downLevelLife > 0){
				pwmCounter++;
				if(pwmCounter >= speed){
					pwmCounter = 0;			
					downLevelLife--;
					downLevel = downLevelMax - (PWM_MAX-downLevelLife) / (PWM_MAX/downLevelMax);
				}
			}else{
				downLevel = 0;
				pwmCounter = 0;
			}
		}
		setPWM((uint8_t)(getBrightness(downLevelLife)));

	}else if(_fullLEDMode == 4){
		// 일정시간 유지
		if(downLevelStay > 0){
			downLevelStay--;
		}else{
			// 시간이 흐르면 레벨을 증가 시킨다.
			if(downLevelLife < PWM_MAX){
				pwmCounter++;
				if(pwmCounter >= speed){
					pwmCounter = 0;			
					downLevelLife++;
					downLevel = downLevelMax - (PWM_MAX-downLevelLife) / (PWM_MAX/downLevelMax);
				}
			}else{
				downLevel = downLevelMax;
				pwmCounter = 0;
			}
		}
		setPWM((uint8_t)(getBrightness(downLevelLife)));

	}
	else
	{
		pwmCounter=0;
		pwmDir=3;
	}
}

//
static void blinkCapsLockLED(void) {
	static int gCounter = 0;
	static int gDelayCounter = 0;
	static uint8_t gLEDState = 1;
	const int gCountMAX = 200;
	uint8_t led = LEDCAPS;
	uint8_t gIsOn = 0;

#ifdef ENABLE_BOOTMAPPER
	if(isBootMapper()){
		gIsOn = 1;
	}
#endif	

	if(gIsOn == 1){
		++gDelayCounter;
		if(gDelayCounter > blinkLedCountDelay){
			gCounter++;
			if(gCounter > gCountMAX){
				if(getLEDState() & LED_STATE_CAPS){	// Caps Lock이 켜져 있을때는 커졌다 켜지고;
					if(gLEDState == 1){
						turnOffLED(led);
					}else{
						turnOnLED(led);
						gDelayCounter = 0;
					}
				}else{	// Caps Lock이 꺼져 있을 때는 켜졌다 꺼진다.
					if(gLEDState == 1){
						turnOnLED(led);
					}else{
						turnOffLED(led);
						gDelayCounter = 0;
					}
				}
				gLEDState ^= 1;
				gCounter = 0;
			}
		}
	}else{
		gCounter = 0;
		gLEDState = 1;
	}
}
static void blinkNumLockLED(void) {
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

#ifndef SCROLL_LOCK_LED_IS_APART
static void blinkScrollLockLED(void) {
	static int counter = 0;
	const int countMAX = 100;
	//on off on off
	if(ledBlinkScrollLockCount > 0){
		counter++;
		if(counter > countMAX){
			if(ledBlinkScrollLockCount == 5 || ledBlinkScrollLockCount == 3){ 
				turnOnLED(LEDCAPS); //PORTLEDS |= (1 << LEDCAPS);
			}else if(ledBlinkScrollLockCount == 4 || ledBlinkScrollLockCount == 2){ 
				turnOffLED(LEDCAPS); //PORTLEDS &= ~(1 << LEDCAPS);
			}else{
				if((getLEDState() & LED_STATE_CAPS)){
					turnOnLED(LEDCAPS);
				}
			}
			counter = 0;

			ledBlinkScrollLockCount--;
		}
	}else{
		counter = 0;
	}
}
#endif

void sleepLED(void){
	// DEBUG_PRINT(("sleepLED \n"));
	turnOffLED(LEDNUM);
	turnOffLED(LEDCAPS);
#ifdef LEDSCROLL
	turnOffLED(LEDSCROLL);
#endif
	setPWM(0);
	turnOffLED(LEDFULLLED);
	_isDidSleep = 1;

}

void wakeUpLED(void){
	// DEBUG_PRINT(("wakeUpLED \n"));
	setLEDIndicate();
	setFullLEDState();
}

void renderLED(void) {
	if(!ledInited) return;
	if(isSleep()){	
		return;
	}

	blinkCapsLockLED();
	blinkNumLockLED();

#ifndef SCROLL_LOCK_LED_IS_APART
	// s/l led
	blinkScrollLockLED();
#endif

	// LED 모드 저장.
	writeLEDMode();

	/* LED Fader */	
	fadePWM();
}
