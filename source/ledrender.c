#ifndef LEDRENDER_C
#define LEDRENDER_C

#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/eeprom.h>

#include <util/delay.h>
#include <string.h>

#include "print.h"
#include "ledrender.h"
#include "hardwareinfo.h"
#include "sleep.h"
#include "keymapper.h"

static uint16_t downLevelStay = 0;
static uint8_t downLevel = 0;
static const uint8_t downLevelMax = 12;
static uint8_t downLevelLife = 0;

static uint8_t ledInited = 0;
static uint8_t LEDstate = 0;     ///< current state of the LEDs
static uint8_t scrollLockLED = 0;

static uint8_t _fullLEDMode = 0;	// 
static uint8_t _fullLEDMode_saved = 0;	// 

static int pwmCounter = 0;
static int pwmDir = 0;
static const uint8_t speed = 8;
static uint8_t _isDidSleep = 0;

static int ledStateCount = 0;

#define PWM_MAX 0xFF
volatile int pwmValue = 0;

static uint8_t _ledBrightnessMax = PWM_MAX; 
static uint8_t _ledBrightnessMax_saved = 0;	// 
static const uint8_t ledBrightnessMin = 30;
static const uint8_t ledBrightnessStep = 25;

static int beyondFNCountDelay = 1800;

static void fadeLED(void);
void setPWM(int xValue);


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
void changeFullLedState(void){
	//LED 모드를 변경한다.
	_fullLEDMode = (_fullLEDMode+1)%5;
	ledStateCount = 1;
	//DEBUG_PRINT(("_fullLEDMode %02x\n ", _fullLEDMode));
	setFullLEDState();
}

void clearLEDInited(void){
	ledInited = 0;
}

void stopTimer(void){  
    TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10)); // No clock source (Timer/Counter stopped).
}

void startTimer(void){
	if(pwmValue == 0) return;
    // 타이머, 카운터1번 컨트롤 분주비
    // TCCR1B |= (1<<CS10); //  clk/1 (From prescaler) 
    // TCCR1B |= (1<<CS11); //  clk/8 (From prescaler) 
    // TCCR1B |= (1<<CS10)|(1<<CS11); // clk/64 (From prescaler)
    TCCR1B |= (1<<CS12); //  clk/256 (From prescaler)     
    // TCCR1B |= (1<<CS10)|(1<<CS12); // clk/1024 (From prescaler)
    
}
void timerProcessCompareB(void){	// like overflow
	turnOnLED(LEDFULLLED);	
	OCR1A = pwmValue;
}
void timerProcessCompareA(void){
	turnOffLED(LEDFULLLED);
}
void setPWM(int xValue){
	static int prevPwmValue = 1;

    pwmValue = xValue;
    if(pwmValue == 0 && prevPwmValue != 0){
        turnOffLED(LEDFULLLED); 
        stopTimer();
    }else if(pwmValue != 0 && prevPwmValue == 0){
        startTimer();
    }
    prevPwmValue = pwmValue;
}

void fadePWM(void){
	fadeLED();
}

void initFullLEDState(void) {

	if(ledInited) return;
	ledInited = 1;

	// timerInit()는 kbd_init()이후에 실행되면 timer2Init()로 인하여 ps/2키보드가 작동하지 않는다.
	// timerInit();	// timerInit() 전체를 실행하면 kbd_init()시 add한 리스너도 지워저 ps2가 동작을 멈춘다.
	// 때문에 리스너는 지우지 않고 init만 실행;
	// timer0Init();	// timer0은 usb일때 led와 연관이 있고 이에 따라 부팅시 작동되지 않는 현상이 있다.

	//timer1Init();
	
	/*#ifdef TCNT2	// support timer2 only if it exists
	timer2Init();	// kbd_init()시 timer2를 사용하므로 이를 초기화하면 안된다.
	#endif*/
	
	// timer1PWMInit(8);
	// timer1PWMBOn();

	timerAttach(TIMER1OUTCOMPAREA_INT, timerProcessCompareA); 
	timerAttach(TIMER1OUTCOMPAREB_INT, timerProcessCompareB); 	// like overflow

	/*
	커스텀 pwm을 위해서 timer를 사용할때 interupt 간격을 좁게 하면 USB의 작동이 잘 안된다.
	// 인터럽트 루틴에서 너무 많은 일을 처리하면 메인 루틴의 처리가 늦어서 일어나는 현상으로 보여진다.
	인터럽트 루틴에서는 최소한의 처리만 하고 나머지는 메인 루틴에 넘기자.
	*/

	// CTC,  TOP : ICR1
    TCCR1A &= ~(1<<WGM10);
    TCCR1A &= ~(1<<WGM11);
    TCCR1B |= (1<<WGM12);
    TCCR1B |= (1<<WGM13);
    ICR1 = PWM_MAX;
    //output compare value B
    OCR1A = 0;
    OCR1B = ICR1;
    // Normal port operation, OC1A/OC1B disconnected.
    TCCR1A &= ~(1<<COM1A1);
    TCCR1A &= ~(1<<COM1A0);
    TCCR1A &= ~(1<<COM1B1);
    TCCR1A &= ~(1<<COM1B0);

    TCNT1 = 0; // 타이머, 카운터1번 카운팅 레지스터 초기화

	TIMSK |= (1<<OCIE1A);
    TIMSK |= (1<<OCIE1B);

    startTimer();

	DEBUG_PRINT(("______________________ init LED State_____________\n"));


	_ledBrightnessMax = eeprom_read_byte((uint8_t *)EEPROM_LED_BRIGHTNESS);
	_ledBrightnessMax_saved = _ledBrightnessMax;

	_fullLEDMode = eeprom_read_byte((uint8_t *)EEPROM_LED_MODE);    // 1바이트 11번지 읽기, 기본값 0xFF ( 255)
	if(_fullLEDMode == 255){
		_fullLEDMode = 0;
	}
	_fullLEDMode_saved = _fullLEDMode;

	setFullLEDState();

	if(INTERFACE == INTERFACE_PS2 || INTERFACE == INTERFACE_PS2_USER){		
		beyondFNCountDelay = beyondFNCountDelay >> 1;	// ps2의 경우 USB보다 대기 시간이 길어서 반으로 줄여줌;
	}

}

void setLEDIndicate(void) {
	 if (LEDstate & LED_STATE_NUM) { // light up num lock
        turnOnLED(LEDNUM);//PORTLEDS |= (1 << LEDNUM);	//
    } else {
        turnOffLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);	//
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
    
	static uint8_t prevSCRLED = 0;
    if ((LEDstate & LED_STATE_SCROLL) && prevSCRLED == 0) { // light up scroll lock
    	
		prevSCRLED = 1;
		// delay 함수를 이용하면 ps2연결시 노이즈가 발생하는지 "방향키 상"이 
		// 연속으로 눌리는 현상이있어, 실시간으로 작동되도록 수정;
    	if(!(LEDstate & LED_STATE_CAPS)){
			scrollLockLED = 5;	//on off on off 1
		}else{
			scrollLockLED = 4;	// off on off 1
		}
		
	} else if(!(LEDstate & LED_STATE_SCROLL) && prevSCRLED == 1){
		
		prevSCRLED = 0;		
		if(!(LEDstate & LED_STATE_CAPS)){
			scrollLockLED = 3;	// on off 1
		}else{
			scrollLockLED = 2;	// off 1
		}
	
	}
	
}

void applyKeyDownForFullLED(void){
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
static void blinkBeyondFNLED(uint8_t xIsBeyondFN) {
	// for beyond fn
	static int beyondFNCounter = 0;
	static int beyondFNDelayCounter = 0;
	static uint8_t beyondFNLEDState = 1;
	const int beyondFNCountMAX = 200;
	uint8_t led = LEDNUM;

#ifdef ENABLE_BOOTMAPPER
	if(isBootMapper()){
		led = LEDCAPS;
		// DEBUG_PRINT(("isBootMapper(): %d \n", isBootMapper()));

		xIsBeyondFN = 1;
	}
#endif

	if(xIsBeyondFN == 1){
		

		beyondFNDelayCounter++;
		if(beyondFNDelayCounter > beyondFNCountDelay){
			beyondFNCounter++;
			if(beyondFNCounter > beyondFNCountMAX){
				if(getLEDState() & LED_STATE_NUM){	// Num Lock이 켜져 있을때는 커졌다 켜지고;
					if(beyondFNLEDState == 1){
						turnOffLED(led);
					}else{
						turnOnLED(led);
						beyondFNDelayCounter = 0;
					}
				}else{	// Num Lock이 꺼져 있을 때는 켜졌다 꺼진다.
					if(beyondFNLEDState == 1){
						turnOnLED(led);
					}else{
						turnOffLED(led);
						beyondFNDelayCounter = 0;
					}
				}
				beyondFNLEDState ^= 1;
				beyondFNCounter = 0;
			}
		}
	}else{
		beyondFNCounter = 0;
		beyondFNLEDState = 1;
	}
}

#ifndef SCROLL_LOCK_LED_IS_APART
static void blinkScrollLockLED(void) {
	// for beyond fn
	static int counter = 0;
	const int countMAX = 100;
	//on off on off
	if(scrollLockLED > 0){
		counter++;
		if(counter > countMAX){
			if(scrollLockLED == 5 || scrollLockLED == 3){ 
				turnOnLED(LEDCAPS); //PORTLEDS |= (1 << LEDCAPS);
			}else if(scrollLockLED == 4 || scrollLockLED == 2){ 
				turnOffLED(LEDCAPS); //PORTLEDS &= ~(1 << LEDCAPS);
			}else{
				//PORTD = backupPORTD;
				if((getLEDState() & LED_STATE_CAPS)){
					turnOnLED(LEDCAPS);
				}
			}
			counter = 0;

			scrollLockLED--;
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
	if(_fullLEDMode == 2 || _fullLEDMode == 4){
		// 켜져 있는 상태일때;
		pwmCounter = 0;
		_isDidSleep = 0;
	}else if(_fullLEDMode == 0 || _fullLEDMode == 3){
		// 꺼져 있는 상태 일때;
		_isDidSleep = 1;
	}else{
		// pwm이 중간 값일때;
		_isDidSleep = 0;		
	}
}

void wakeUpLED(void){
	// DEBUG_PRINT(("wakeUpLED \n"));
	setLEDIndicate();
	setFullLEDState();
}

void doSleep(void){
	if(_isDidSleep == 1) return;
	//DEBUG_PRINT(("pwmCounter %d \n", (uint16_t)(PWM_MAX-pwmCounter/speed)));
	setPWM((uint8_t)(getBrightness(PWM_MAX-pwmCounter/speed)));
	if(pwmCounter >= PWM_MAX*speed) {		
		setPWM(0);
		turnOffLED(LEDFULLLED);
		_isDidSleep = 1;
	}
	pwmCounter++;
}

void renderLED(uint8_t xIsBeyondFN) {
	if(!ledInited) return;
	if(isSleep()){		
		doSleep();
		return;
	}
	
	// for beyond fn
	blinkBeyondFNLED(xIsBeyondFN);

#ifndef SCROLL_LOCK_LED_IS_APART
	// s/l led
	blinkScrollLockLED();
#endif

	// LED 모드 저장.
	writeLEDMode();

	/* LED Fader */	
	fadePWM();
}
#endif