
#include "timerinclude.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdbool.h>
#include <avr/eeprom.h>

#include <util/delay.h>
#include <string.h>

#include "ledconfig.h"
#include "ledrender.h"

#include "eeprominfo.h"
#include "ledconfig.h"
#include "fncontrol.h"
#include "sleep.h"
#include "keymapper.h"
#include "main.h"
#include "bootmapper.h"
#include "ps2avru_util.h"
#include "oddebug.h"
#include "i2c/i2c.h"        // include i2c support
#include "led2.h"
#include "keydownbuffer.h"

#include "ledrender_inc.c"

#define PWM_MAX 			0xFF
#define FULL_LED_MODE_NUM 	5
#define FULL_LED_MODE2_NUM 	5
#define FULL_LED_MODE2_KEY_EVENT_NUM 	3

#define PRESS_MODE_UP	 	0
#define PRESS_MODE_DOWN	 	1
#define PRESS_MODE_CHANGE 	2

#define SAVE_BIT_LED_MODE			0
#define SAVE_BIT_LED_BRITNESS_MAX	1
#define SAVE_BIT_LED2_BRITNESS_MAX	2
#define SAVE_BIT_LED2_NUM			3
#define SAVE_BIT_LED2_COLOR1		4
#define SAVE_BIT_LED2_COLOR2		5
#define SAVE_BIT_LED2_COLOR3		6
#define SAVE_BIT_LED2_COLOR_RAINBOW	7

#define SAVE2_BIT_LED2_KEY_EVENT	0
#define SAVE2_BIT_LED2_COLOR_KEY1	1
#define SAVE2_BIT_LED2_FADE_TYPE	2

static uint8_t _saved = 0;	//
static uint8_t _saved2 = 0;	//
//static bool confirmLed2Save = false;	//

static int downLevelStay = 0;
static uint8_t downLevel = 0;
static const uint8_t downLevelMax = 12;
static uint8_t downLevelLife = 0;

static uint8_t ledInited = 0;

static uint8_t _fullLEDMode = 0;	// 
static uint8_t _Led2Mode = 0;	//
static uint8_t _Led2KeyEventMode = 0;	//
static uint8_t _Led2KeyEventType = 0;	//

static uint8_t pwmCounter = 0;
static uint8_t pwmDir = 0;
#ifndef PWM_SPEED
	#define PWM_SPEED 8
#endif

static int ledStateCount = 0;
static cRGB_t _currentLedAllColor;

static uint8_t _ledBrightnessMax = PWM_MAX;
static uint8_t ledBrightnessLimit;
static const uint8_t ledBrightnessMin = 30;
static const uint8_t ledBrightnessStep = 25;

static void fadeLED(void);
static void fadeLED2(void);
static void setFullLedState(void);
static void setLed2State(void);
static uint16_t getBrightness(uint16_t xValue);

// i2c
//#define LOCAL_ADDR  0xA0
#define TARGET_ADDR 0xB0

#define led2TickPerFrame    40
#define led2SpeedValue      1
//#define LED2_MAX_400MA      186	// each color
//#define LED2_MAX_200MA      73
//#define LED2_MAX_GAP		113	//(LED2_MAX_400MA - LED2_MAX_200MA)
#define LED2_MAX_400MA_SUM	5580	//6600	//5580	//(LED2_MAX_400MA*3)	// 3 color , 10EA (default numOfLeds)
#define LED2_MAX_200MA_SUM	2190	//3300	//2190	//(LED2_MAX_200MA*3)
//#define LED2_MAX_GAP_SUM	3000	//3300	//3390	//(LED2_MAX_400MA_SUM - LED2_MAX_200MA_SUM)

#define LED2_COLOR_MAX  	0xFF		// const
#define LED_SUM_MIN       	50
#define LED_NUM_MAX       	24
#define LED_NUM_DEFAULT     12

#define LENGTH_OF_RAINBOW_COLOR     7

static uint8_t numOfLeds = LED_NUM_DEFAULT; //LED_NUM_MAX;
//static cRGB_t led[LED_NUM_MAX];
static cRGB_t ledModified[LED_NUM_MAX];
static cRGB_t rainbowColor[7];
static uint16_t led2Brightness; 	// = LED2_MAX_200MA_SUM;
static uint16_t led2SumLimit; 	// = LED2_MAX_400MA_SUM / numOfLeds;
static u08 i2cLength;
static int led2MaxSum400ma;
static int led2MaxSum200ma;
//static int led2MaxSumGap;
static cRGB_t color1;
static cRGB_t color2;
static cRGB_t color3;
static cRGB_t color_key1;

static cRGB_t prevRgb;

static uint8_t _pressed = PRESS_MODE_UP;
static bool _ledOff = false;
static uint8_t _rainbowMode = 0;
static uint8_t _changeCount = 0;
static uint8_t _delayCount = 0;

static uint8_t _rainbowIndex = 0;
static uint8_t arrived;

static void setLed2All(cRGB_t xRgb);
static void _setLed2All(cRGB_t xRgb);
static void getMaxRgbValue(cRGB_t *xRgb);
static void turnOffLed2(void);
static void sendI2c(void);
static void setLedBalance(void);
static void setLed2BrightnessLimit(void);
static void setLed2BrightnessAfter(void);

void initLED(void){
	// led pin
	DDRD |= (LEDNUM | LEDCAPS | LEDFULLLED | LEDSCROLL);	// output;
	PORTD &= ~(LEDNUM | LEDCAPS | LEDFULLLED | LEDSCROLL);	// low

	// initialize i2c function library
    i2cInit();
    i2cSetBitrate(400);

//    uint8_t len = sizeof(led2_info_t);
//    DBG1(0xBF, (uchar *)&len, 1);
}

void setLed2Num(uint8_t xNum){
	// num = 5 : led2 sub control size(15byte);
	if(xNum == 0) return;

	numOfLeds = xNum;
	if(numOfLeds == 0xFF) {numOfLeds = LED_NUM_DEFAULT;}
	else if(numOfLeds > LED_NUM_MAX) {numOfLeds = LED_NUM_MAX;}

	DBG1(0xD9, (uchar *)&numOfLeds, 1);

	led2MaxSum400ma = LED2_MAX_400MA_SUM / numOfLeds;
	if(led2MaxSum400ma > 762) led2MaxSum400ma = 762;	// 255 * 3
	led2MaxSum200ma = LED2_MAX_200MA_SUM / numOfLeds;
	if(led2MaxSum200ma > 762) led2MaxSum200ma = 762;
//	led2MaxSumGap = led2MaxSum400ma - led2MaxSum200ma;
	DBG1(0xD4, (uchar *)&led2MaxSum400ma, 2);
	DBG1(0xD2, (uchar *)&led2MaxSum200ma, 2);

    if(INTERFACE == INTERFACE_PS2 || INTERFACE == INTERFACE_PS2_USER){
    	led2SumLimit = led2MaxSum200ma;
    }else{
    	led2SumLimit = led2MaxSum400ma;
    }
    setLed2BrightnessLimit();
}

// led2.h
// data = hid report
static uint8_t led2rainbowSettingCount = 0;

void getLed2(led2_info_t *buffer){

	// report led2 info
	// num 1byte, mode 1byte, brightness 1byte, color1 3byte, color2 3byte, color3 3byte, rainbow colors 21byte, key color 3byte
	buffer->num = numOfLeds;
	buffer->mode = _Led2Mode;
	buffer->brightness = (uint8_t)(led2Brightness / 3);
	buffer->color1 = color1;
	buffer->color2 = color2;
	buffer->color3 = color3;
	uint8_t i;
	for(i=0;i<7;++i){
		buffer->rainbow[i] = rainbowColor[i];
	}
	buffer->keymode = _Led2KeyEventMode;
	buffer->colorkey1 = color_key1;
	buffer->fadetype = _rainbowMode;

//	DBG1(0x22, 0, 0);
//	DBG1(0xC2, (uchar *)buffer, sizeof(led2_info_t));
}

void __setLed2Mode(uint8_t xMode, uint8_t xKeyEventMode){
	_Led2Mode = xMode;
	if(_Led2Mode >= FULL_LED_MODE2_NUM) _Led2Mode = 0;

	if(xKeyEventMode == 0 ){	// 키 이벤트가 없을 경우에만 모드 변경 적용;
		_pressed = PRESS_MODE_UP;
		setLed2State();
	}else{
		_pressed = PRESS_MODE_CHANGE;	// up event로 led 컨트롤을 미룬다.
	}

	_saved |= BV(SAVE_BIT_LED_MODE);
}
void setLed2(uint8_t *data){
	/*
	 * data[0] = report id;
	 * data[1] = index
	 * data[2] ~ = data...
	 */
	switch (*(data+1)){
/*	case LED2_INDEX_LED_SAVE:
		confirmLed2Save = true;
		break;*/
	case LED2_INDEX_COLOR_RAINBOW_INIT:
		led2rainbowSettingCount = 0;
		break;
	case LED2_INDEX_COLOR_RAINBOW:
		if(led2rainbowSettingCount > 0 && led2rainbowSettingCount < 4){
			rainbowColor[(led2rainbowSettingCount*2)-2] = *((cRGB_t *)(data+2));
			rainbowColor[(led2rainbowSettingCount*2)-1] = *((cRGB_t *)(data+5));
		}else if(led2rainbowSettingCount==4){
			rainbowColor[6] = *((cRGB_t *)(data+2));

//			DBG1(0xbb, (uchar *)data, 8);

			_saved |= BV(SAVE_BIT_LED2_COLOR_RAINBOW);
			break;
		}
		++led2rainbowSettingCount;

		break;
	case LED2_INDEX_COLOR_SET1:
		color1 = *((cRGB_t *)(data+2));
		_saved |= BV(SAVE_BIT_LED2_COLOR1);

	    setLed2State();
		break;
	case LED2_INDEX_COLOR_SET2:
		color2 = *((cRGB_t *)(data+2));
		_saved |= BV(SAVE_BIT_LED2_COLOR2);

	    setLed2State();
		break;
	case LED2_INDEX_COLOR_SET3:
		color3 = *((cRGB_t *)(data+2));
		_saved |= BV(SAVE_BIT_LED2_COLOR3);

	    setLed2State();
		break;
	case LED2_INDEX_COLOR_KEY_SET1:
		color_key1 = *((cRGB_t *)(data+2));
		_saved2 |= BV(SAVE2_BIT_LED2_COLOR_KEY1);

		break;
	case LED2_INDEX_MODE:
		__setLed2Mode(*(data+2), 0);	// 항상 led 재설정;
		break;
	case LED2_INDEX_LED_BRIGHTNESS:
		led2Brightness = *(data+2) * 3;
		if(led2Brightness == 0) led2Brightness = 3;
		setLed2BrightnessLimit();

		setLed2BrightnessAfter();
		setLed2State();
		break;
	case LED2_INDEX_MODE_KEY:
		_Led2KeyEventMode = *(data+2);
		_saved2 |= BV(SAVE2_BIT_LED2_KEY_EVENT);
		break;
	case LED2_INDEX_MODE_FADE_TYPE:
		_rainbowMode = *(data+2);
		_delayCount = 0;
		_changeCount = 0;
		_rainbowIndex = 0;
		_saved2 |= BV(SAVE2_BIT_LED2_FADE_TYPE);
		break;
	/*case LED2_INDEX_MODE_KEY_TYPE:
		break;*/
	case LED2_INDEX_LED_NUM:
		turnOffLed2();
		setLed2Num(*(data+2));
	    setLedBalance();

		_saved |= BV(SAVE_BIT_LED2_NUM);
		break;
	/*case LED2_INDEX_LED_ENABLED:
		turnOffLed2();
		setLed2Num(*(data+2));
	    setLedBalance();

		_saved |= BV(SAVE_BIT_LED2_NUM);
		break;*/
	}

	ledStateCount = 0;
}

static void changeLed2KeyEventMode(void){
//	_Led2KeyEventMode = (_Led2KeyEventMode + 1);
	if(++_Led2KeyEventMode == FULL_LED_MODE2_KEY_EVENT_NUM) _Led2KeyEventMode = 0;
	_saved2 |= BV(SAVE2_BIT_LED2_KEY_EVENT);
}

void stopFullLed(void){
    // stop timer for usb report

	timer1SetPrescaler( TIMERRTC_CLK_STOP );	// set prescaler
	cbi(TIMSK, TOIE1);
	setPWM(0);
	turnOffLED(LEDFULLLED);
	timer1PWMBOff();
}
void startFullLed(void){
	timer1Init();
	timer1PWMBOn();
	if(_fullLEDMode == 2){
		setFullLedState();
	}
}

/*
 * 전류 배분;
 *
 * 하판은 200~400mA (LED2_SUM_MAX 값으로 93 * 3 ~ 186 * 3)
 * 하판이 200이하 경우 상판은 200(= limit 255)
 * 400일 경우는 25mA(= limit 50)
 */
static void setLedBalance(void){
	// 50~ 255
	if(led2MaxSum200ma  > led2Brightness|| led2MaxSum400ma == led2MaxSum200ma) {
		ledBrightnessLimit = 255;
		return;
	}
	uint16_t gLim;
	// 50 + (205 * (610) / 330);
	gLim = 50 + (205 * (uint32_t)(led2MaxSum400ma  - led2Brightness) / (led2MaxSum400ma - led2MaxSum200ma));
	ledBrightnessLimit = (uint8_t)gLim;
}

static void setLed2BrightnessLimit(void){
	if(led2Brightness > led2SumLimit) led2Brightness = led2SumLimit;
}

void initFullLEDStateAfter(void){

	_saved = 0;	// nothing any update
	_saved2 = 0;

	_ledBrightnessMax = eeprom_read_byte((uint8_t *)EEPROM_LED_BRIGHTNESS);

	// 1바이트 11번지 읽기, 기본값 0xFF ( 255)
	// 0xF0 : led2, 0x0F : full led
	_fullLEDMode = eeprom_read_byte((uint8_t *)EEPROM_LED_MODE) & 0x0F;
	if(_fullLEDMode == 0x0F){
		_fullLEDMode = 0;
	}

	// led2
	_Led2Mode = (eeprom_read_byte((uint8_t *)EEPROM_LED_MODE) >> 4) & 0x0F;
	if(_Led2Mode == 0x0F){
		_Led2Mode = 0;
	}

	// led2 color1~3
	color1.g = eeprom_read_byte((uint8_t *)EEPROM_LED2_COLOR_1);
	color1.r = eeprom_read_byte((uint8_t *)(EEPROM_LED2_COLOR_1+1));
	color1.b = eeprom_read_byte((uint8_t *)(EEPROM_LED2_COLOR_1+2));
	color2.g = eeprom_read_byte((uint8_t *)EEPROM_LED2_COLOR_2);
	color2.r = eeprom_read_byte((uint8_t *)(EEPROM_LED2_COLOR_2+1));
	color2.b = eeprom_read_byte((uint8_t *)(EEPROM_LED2_COLOR_2+2));
	color3.g = eeprom_read_byte((uint8_t *)EEPROM_LED2_COLOR_3);
	color3.r = eeprom_read_byte((uint8_t *)(EEPROM_LED2_COLOR_3+1));
	color3.b = eeprom_read_byte((uint8_t *)(EEPROM_LED2_COLOR_3+2));

	// set rainbow color

	// led2 rainbow
	uint8_t i;
	for(i=0;i<LENGTH_OF_RAINBOW_COLOR;++i){
		rainbowColor[i].g = eeprom_read_byte((uint8_t *)EEPROM_LED2_COLOR_RAINBOW+(i*3));
		rainbowColor[i].r = eeprom_read_byte((uint8_t *)(EEPROM_LED2_COLOR_RAINBOW+(i*3)+1));
		rainbowColor[i].b = eeprom_read_byte((uint8_t *)(EEPROM_LED2_COLOR_RAINBOW+(i*3)+2));
	}
	// led2 rainbow fading type
	_rainbowMode = eeprom_read_byte((uint8_t *)EEPROM_LED2_FADE_TYPE);

	// led2 num
	led2Brightness = eeprom_read_byte((uint8_t *)EEPROM_LED2_BRIGHTNESS);
	led2Brightness = (led2Brightness == 0xFF ? 0x4F : led2Brightness) * 3;
	setLed2Num(eeprom_read_byte((uint8_t *)EEPROM_LED2_NUM));

	// led2 key mode
	_Led2KeyEventMode = eeprom_read_byte((uint8_t *)EEPROM_LED2_KEY_EVENT) & 0x0F;	// 0xF0 : type, 0x0F : mode
	if(_Led2KeyEventMode == 0x0F){
		_Led2KeyEventMode = 0;
	}
	// led2 key color 1
	color_key1.g = eeprom_read_byte((uint8_t *)EEPROM_LED2_COLOR_KEY1);
	color_key1.r = eeprom_read_byte((uint8_t *)(EEPROM_LED2_COLOR_KEY1+1));
	color_key1.b = eeprom_read_byte((uint8_t *)(EEPROM_LED2_COLOR_KEY1+2));


    setLedBalance();

	setFullLedState();
	setLed2State();
}


static void setFullLedState(void) {
	/* _fullLEDMode
		0 = off
		1 = fading
		2 = on
		3 = key down level up
		4 = key down level down
	*/
	switch(_fullLEDMode) {
		case 0:
			setPWM(0);
			break;
		case 1:
			pwmCounter = 0;
			pwmDir = 0;
			break;
		case 2:
			setPWM(getBrightness(PWM_MAX));
			break;
		case 3:
		case 4:
			downLevel = 3;
			if(_fullLEDMode == 4) downLevel = 1; 	// 3으로 시작하면 효과가 별로 안 남.
			downLevelLife = downLevel * PWM_MAX / downLevelMax;
			break;
	}

}

static void applyStaticFullLed(void){
	if(_fullLEDMode == 2){
		setFullLedState();
	}
}

static void setLed2BrightnessAfter(void){

	setLedBalance();

	DBG1(0x11, (uchar *)&led2Brightness, 2);

	if(_Led2Mode == 2 || _Led2Mode == 3 || _Led2Mode == 4){
		_pressed = PRESS_MODE_CHANGE;
//		setLed2State();
	}
	applyStaticFullLed();
	_saved |= BV(SAVE_BIT_LED2_BRITNESS_MAX);
}

void increaseLedBrightness(uint8_t xFullLedMode){
	if(xFullLedMode == FULL_LED_MODE1){

		if(_ledBrightnessMax == PWM_MAX) return;

		int gLedBrightness = _ledBrightnessMax + ledBrightnessStep;
		if(gLedBrightness > PWM_MAX){
			_ledBrightnessMax = PWM_MAX;
		}else{
			_ledBrightnessMax = gLedBrightness;
		}
		applyStaticFullLed();
		_saved |= BV(SAVE_BIT_LED_BRITNESS_MAX);
	}else if(xFullLedMode == FULL_LED_MODE2){
		led2Brightness = led2Brightness + 100;
		setLed2BrightnessLimit();

		setLed2BrightnessAfter();
	}
	ledStateCount = 0;
}
void reduceLedBrightness(uint8_t xFullLedMode){
	if(xFullLedMode == FULL_LED_MODE1){
		if(_ledBrightnessMax == ledBrightnessMin) return;

		_ledBrightnessMax = _ledBrightnessMax - ledBrightnessStep;
		if(_ledBrightnessMax < ledBrightnessMin){
			_ledBrightnessMax = ledBrightnessMin;
		}
		applyStaticFullLed();
		_saved |= BV(SAVE_BIT_LED_BRITNESS_MAX);
	}else if(xFullLedMode == FULL_LED_MODE2){
		if(led2Brightness <= 100) {
			led2Brightness = LED_SUM_MIN;
		}else{
			led2Brightness = led2Brightness - 100;
		}
		if(led2Brightness < LED_SUM_MIN) led2Brightness = LED_SUM_MIN;
		setLed2BrightnessAfter();
	}
	ledStateCount = 0;
}
void changeFullLedState(uint8_t xFullLedMode){
	if(isSleep()){
		return;
	}

	if(xFullLedMode == FULL_LED_MODE1){
		//LED 모드를 변경한다.
		_fullLEDMode = (_fullLEDMode+1);
		if(_fullLEDMode == FULL_LED_MODE_NUM) _fullLEDMode = 0;
		setFullLedState();
		_saved |= BV(SAVE_BIT_LED_MODE);
	}else if(xFullLedMode == FULL_LED_MODE2){
		__setLed2Mode(_Led2Mode+1, _Led2KeyEventMode);	// 키보드를 이용할 경우 키이벤트에 따라 재설정 판가름;
	}
	ledStateCount = 0;
}

void clearLEDInited(void){
	ledInited = 0;
}

void setLEDState(uint8_t xState){
	LEDstate = xState;
}
uint8_t getLEDState(void){
	return LEDstate;
}
void setLEDIndicate(void) {
	static uint8_t prevLEDstate;

	if(isBeyondFnLedEnabled() == BEYOND_FN_LED_NL){
		getLedBlink(LED_STATE_NUM, isBeyondFN(), &prevLEDstate, &ledBlinkCount);
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
    if(isBeyondFnLedEnabled() == BEYOND_FN_LED_SL){
		getLedBlink(LED_STATE_SCROLL, isBeyondFN(), &prevLEDstate, &ledBlinkCount);
	}else{
		if (LEDstate & LED_STATE_SCROLL) { // light up scroll lock
			turnOnLED(LEDSCROLL); //PORTLEDS |= (1 << LEDCAPS);	//
		} else {
			turnOffLED(LEDSCROLL); //PORTLEDS &= ~(1 << LEDCAPS);	//
		}
	}

	prevLEDstate = LEDstate;

}

static void writeLEDMode(void) {
	// LED 모드 저장, 너무많은 eeprom접근을 막기위해 일정 간격으로 저장한다.
//	const int countMAX = 1000;
	if((_saved > 0 || _saved2 > 0) && ++ledStateCount == 3000){
		ledStateCount = 0;

		// full led
		if(((_saved >> SAVE_BIT_LED_MODE) & 0x01)){	// || ((_saved >> SAVE_BIT_LED2_MODE) & 0x01)){
			eeprom_write_byte((uint8_t *)EEPROM_LED_MODE, (_fullLEDMode & 0x0F) | ((_Led2Mode & 0x0F) << 4));
		}

		// brightness
		if(((_saved >> SAVE_BIT_LED_BRITNESS_MAX) & 0x01)){
			eeprom_write_byte((uint8_t *)EEPROM_LED_BRIGHTNESS, _ledBrightnessMax);
		}
		if(((_saved >> SAVE_BIT_LED2_BRITNESS_MAX) & 0x01)){
			eeprom_write_byte((uint8_t *)EEPROM_LED2_BRIGHTNESS, led2Brightness/3);
		}

		// led num SAVE_BIT_LED2_NUM
		if(((_saved >> SAVE_BIT_LED2_NUM) & 0x01)){
//			prevRgb = (cRGB_t){0,0,0};
			setFullLedState();
		    setLed2State();

			eeprom_write_byte((uint8_t *)EEPROM_LED2_NUM, numOfLeds);
		}

		// led2 color1~3
		if(((_saved >> SAVE_BIT_LED2_COLOR1) & 0x01)){
			eeprom_write_byte((uint8_t *)(EEPROM_LED2_COLOR_1), color1.g);
			eeprom_write_byte((uint8_t *)(EEPROM_LED2_COLOR_1+1), color1.r);
			eeprom_write_byte((uint8_t *)(EEPROM_LED2_COLOR_1+2), color1.b);
		}
		if(((_saved >> SAVE_BIT_LED2_COLOR2) & 0x01)){
			eeprom_write_byte((uint8_t *)(EEPROM_LED2_COLOR_2), color2.g);
			eeprom_write_byte((uint8_t *)(EEPROM_LED2_COLOR_2+1), color2.r);
			eeprom_write_byte((uint8_t *)(EEPROM_LED2_COLOR_2+2), color2.b);
		}
		if(((_saved >> SAVE_BIT_LED2_COLOR3) & 0x01)){
			eeprom_write_byte((uint8_t *)(EEPROM_LED2_COLOR_3), color3.g);
			eeprom_write_byte((uint8_t *)(EEPROM_LED2_COLOR_3+1), color3.r);
			eeprom_write_byte((uint8_t *)(EEPROM_LED2_COLOR_3+2), color3.b);
		}

		// led2 rainbow
		if(((_saved >> SAVE_BIT_LED2_COLOR_RAINBOW) & 0x01)){
			uint8_t *gRainbowData = (uint8_t *)rainbowColor;
			uint8_t i;
			for(i= 0; i < 21; ++i){ // LENGTH_OF_RAINBOW_COLOR * 3
				eeprom_write_byte((uint8_t *)(EEPROM_LED2_COLOR_RAINBOW+i), *gRainbowData++);
			}
//			DBG1(0x22, (uint8_t *)rainbowColor, 21);
		}

		// led2 rainbow fading type
		if(((_saved2 >> SAVE2_BIT_LED2_FADE_TYPE) & 0x01)){
			eeprom_write_byte((uint8_t *)EEPROM_LED2_FADE_TYPE, _rainbowMode);
		}

		// led2 key color1
		if(((_saved2 >> SAVE2_BIT_LED2_KEY_EVENT) & 0x01)){
			eeprom_write_byte((uint8_t *)EEPROM_LED2_KEY_EVENT, (_Led2KeyEventMode & 0x0F) | ((_Led2KeyEventType & 0x0F) << 4));
		}

		if(((_saved2 >> SAVE2_BIT_LED2_COLOR_KEY1) & 0x01)){
			eeprom_write_byte((uint8_t *)(EEPROM_LED2_COLOR_KEY1), color_key1.g);
			eeprom_write_byte((uint8_t *)(EEPROM_LED2_COLOR_KEY1+1), color_key1.r);
			eeprom_write_byte((uint8_t *)(EEPROM_LED2_COLOR_KEY1+2), color_key1.b);
		}

		_saved = 0;
		_saved2 = 0;
		blinkOnce(50);
	}
}

static uint16_t getBrightness(uint16_t xValue){
	uint16_t gBri = xValue * _ledBrightnessMax / PWM_MAX;
	gBri = gBri > ledBrightnessLimit ? ledBrightnessLimit : gBri;
//	DBG1(0x12, (void *)&gBri, 2);
	return gBri;
}

static void fadePWM(void){
	fadeLED();
	fadeLED2();
}

//TODO: led2
/*
 * 3개의 LED를 사용하기 때문에 3개 모두의 값이 제한 값이 초과 되지 않도록 하고,
 * 2개 이하의 LED를 이용할 경우 개별 값은 LED2_COLOR_MAX까지 설정할 수 있도록 하여 밝기를 확보한다.
 */
static void getMaxRgbValue(cRGB_t *xRgb){
	uint16_t gSum = xRgb->r + xRgb->g + xRgb->b;
	uint32_t r,g,b;
	if(gSum > led2Brightness){
		r = xRgb->r;
		g = xRgb->g;
		b = xRgb->b;
		r = r * led2Brightness / gSum;
		g = g * led2Brightness / gSum;
		b = b * led2Brightness / gSum;
		xRgb->r = r;
		xRgb->g = g;
		xRgb->b = b;
	}
}

static void turnOffLed2(void){
	ledModified[0].r = 0;
	ledModified[0].g = 0;
	ledModified[0].b = 0;
	prevRgb = (cRGB_t){0,0,0};
	_currentLedAllColor = ledModified[0];
	_setLed2All(_currentLedAllColor);
}


void applyKeyDownForFullLED(uint8_t keyidx, uint8_t col, uint8_t row, uint8_t isDown){

	// for test sub leds
/*if(isDown){
	ledModified[0].g = 0x03;
}else{
	ledModified[0].g = 0x00;
}
	i2cLength = 1;
	sendI2c();*/
	if(isSleep() || _ledOff){
		return;
	}

	if(isDown){
		// full led
		if(_fullLEDMode > 2){
			// 키를 누르면 값을 증가 시킨다.
			downLevelStay = 500; //511;
			if(downLevel < downLevelMax && _fullLEDMode == 3){
				downLevel++;
			}else
			if(downLevel > 0 && _fullLEDMode == 4){
				downLevel--;
			}
			downLevelLife = (uint16_t)(downLevel * PWM_MAX) / downLevelMax;
		}

		// led2
		if(_Led2Mode > 0 && keyidx != KEY_LED_ON_OFF){
			if(_Led2KeyEventMode == 1){		// color 1
				ledModified[0] = color_key1;
				setLed2All(ledModified[0]);

				_pressed = PRESS_MODE_DOWN;
			}else if(_Led2KeyEventMode == 2){	// complementary color
				ledModified[0] = (cRGB_t){255 - _currentLedAllColor.g, 255 - _currentLedAllColor.r, 255 - _currentLedAllColor.b};
				setLed2All(ledModified[0]);

				_pressed = PRESS_MODE_DOWN;
			}
		}
	}else{	// up
		// led2
		if(_pressed == PRESS_MODE_DOWN){
			setLed2All(_currentLedAllColor);
		}else if(_pressed == PRESS_MODE_CHANGE){
			// led2 mode에 변동이 있다.
			setLed2State();
		}
		_pressed = PRESS_MODE_UP;
	}


}

static void setLed2State(void){
	/* _fullLEDMode2
		0 = off
		1 = rainbow fading
		2 = rgb on
		3 = shuffle
		// 3 = key down effect
	*/
//	uint8_t i;
	DBG1(0xAE, (uchar *)&_Led2Mode, 1);

	if((INTERFACE == INTERFACE_PS2 || INTERFACE == INTERFACE_PS2_USER)){
		if(_Led2Mode == 0)
			startFullLed();
		else
			stopFullLed();
    }

	switch(_Led2Mode) {
		case 0:
			turnOffLed2();
			ledBrightnessLimit = 255;
			break;
		case 1:
			setLedBalance();
			_delayCount = 0;
			_changeCount = 0;
			break;
		case 2:
			setLedBalance();
			_currentLedAllColor = color1;
			setLed2All(_currentLedAllColor);
			break;
		case 3:
			setLedBalance();
			_currentLedAllColor = color2;
			setLed2All(_currentLedAllColor);
			break;
		case 4:
			setLedBalance();
			_currentLedAllColor = color3;
			setLed2All(_currentLedAllColor);
			break;
	}

	applyStaticFullLed();

}
static void sendI2c(void){
	i2cMasterSendNI(TARGET_ADDR, i2cLength, (u08 *)ledModified);
	
	i2cLength = 0;
}

static void _setLed2All(cRGB_t xRgb){
	uint8_t i;

	for (i=0; i<numOfLeds; i++){
		ledModified[i] = xRgb;

	    i2cLength = i2cLength + 3;
	}

	sendI2c();

	/*
	// for test sub leds
	uint8_t i;
	ledModified[0].g = 0x01;
	ledModified[0].r = 0x00;
	ledModified[0].b = 0x00;
	i2cLength = 6;

	for (i=0; i<3; i++){
		ledModified[i+2] = xRgb;
		i2cLength = i2cLength + 3;
	}
	sendI2c();*/
}

static void getRainbowColorByIndex(cRGB_t *pRgb, uint8_t *pIndex, uint8_t aOffset){	
	uint8_t kIndex = (*pIndex + aOffset) % LENGTH_OF_RAINBOW_COLOR;

	if(rainbowColor[kIndex].r == 0 && rainbowColor[kIndex].g == 0 && rainbowColor[kIndex].b == 0)
	// if(rainbowColor[kIndex] == 0)
	{
		*pIndex = *pIndex + 1;
		getRainbowColorByIndex(pRgb, pIndex, aOffset);
	}
	else
	{
		*pRgb = rainbowColor[kIndex];
	}
} 

static void increaseRainbowIndex(void)
{
	if (++_rainbowIndex >= LENGTH_OF_RAINBOW_COLOR) {
		_rainbowIndex = 0;
	}
}

static void setNextAvailableRainbowIndex(uint8_t *aIndex)
{
	uint8_t kIndex;
	uint8_t i;
	for (i = 0; i < numOfLeds; i++) {					
		kIndex = (*aIndex + _rainbowIndex) % LENGTH_OF_RAINBOW_COLOR;
		if(rainbowColor[kIndex].r == 0 
			&& rainbowColor[kIndex].g == 0 
			&& rainbowColor[kIndex].b == 0)
		{
			*aIndex	+= 1;
		}
		else
		{
			break;
		}
	}
}

static void setLed2All(cRGB_t xRgb){

//DBG1(0x40, (uchar *)&xRgb, 3);
	getMaxRgbValue(&xRgb);
//DBG1(0x44, (uchar *)&xRgb, 3);

	if(prevRgb.r == xRgb.r && prevRgb.g == xRgb.g && prevRgb.b == xRgb.b) return;
	prevRgb = xRgb;
//DBG1(0x45, (uchar *)&xRgb, 3);

	_setLed2All(xRgb);
}

#define VALUE_RAINBOW_FLOW 		2
static void _fadeLED2(void){
	static int gFrame = 0;

	if(++gFrame >= led2TickPerFrame){	// setDelay(led2TickPerFrame)){
		gFrame = 0;
		uint8_t i;
		if (_Led2Mode == 1) { // 1 = rainbow fading

			/*if (_rainbowMode == 1) {	// immediately
				if (_delayCount++ == 0) {
					_currentLedAllColor = rainbowColor[_rainbowIndex];
					setLed2All(_currentLedAllColor);
					increaseRainbowIndex();
				}
			} else if (_rainbowMode == 2) {	// sequential
				if (_changeCount < numOfLeds) {
					for (i = 0; i < numOfLeds; i++) {
						if (i > _changeCount) {
							// old color
							ledModified[i] = _currentLedAllColor;
						} else {
							// new color
							ledModified[i] = rainbowColor[_rainbowIndex];
							getMaxRgbValue(&ledModified[i]);
						}

						i2cLength = i2cLength + 3;
					}
					++_changeCount;

					sendI2c();
				} else {
					_currentLedAllColor = rainbowColor[_rainbowIndex];
					getMaxRgbValue(&_currentLedAllColor);
				}

				if (++_delayCount == 0) {
					_changeCount = 0;
					increaseRainbowIndex();
				}
			} else if (_rainbowMode == 4) {	// flow
				if (_delayCount++ == 0) {
					_delayCount = 225;
					_changeCount = 0;
					uint8_t kIndex;
					for (i = 0; i < numOfLeds; i++) {					
						kIndex = _rainbowIndex;
						//if(rainbowColor[_rainbowIndex] == 0)
						if(rainbowColor[kIndex].r == 0 
							&& rainbowColor[kIndex].g == 0 
							&& rainbowColor[kIndex].b == 0)
						{
							increaseRainbowIndex();						
						}
						else
						{
							break;
						}
					}

					for (i = 0; i < numOfLeds; i++) {
						// ledModified[i] = rainbowColor[(i+_rainbowIndex) % LENGTH_OF_RAINBOW_COLOR];
						getRainbowColorByIndex(&ledModified[i], &_changeCount, _rainbowIndex);
						getMaxRgbValue(&ledModified[i]);
						i2cLength = i2cLength + 3;
						_changeCount++;
					}

					increaseRainbowIndex();

					sendI2c();
				}
			}else */ if (_rainbowMode == 1) {	// static
				if (_delayCount++ == 0) {
					_changeCount = 0;
					for (i = 0; i < numOfLeds; i++) {
						_changeCount++;
						setNextAvailableRainbowIndex(&_changeCount);
						ledModified[i] = rainbowColor[_changeCount % LENGTH_OF_RAINBOW_COLOR];
						getMaxRgbValue(&ledModified[i]);
						i2cLength = i2cLength + 3;
					}
					sendI2c();
				}
			} else if (_rainbowMode == 2) {	// flow
				uint8_t kIndex;
				_changeCount = 0;
				for (i = 0; i < numOfLeds; i++) {
					_changeCount++;
					setNextAvailableRainbowIndex(&_changeCount);
					kIndex = (_changeCount + _rainbowIndex) % LENGTH_OF_RAINBOW_COLOR;
					if (rainbowColor[kIndex].r > ledModified[i].r + VALUE_RAINBOW_FLOW) {
						ledModified[i].r += VALUE_RAINBOW_FLOW;
					} else if (rainbowColor[kIndex].r < ledModified[i].r - VALUE_RAINBOW_FLOW) {
						ledModified[i].r -= VALUE_RAINBOW_FLOW;
					}

					if (rainbowColor[kIndex].g > ledModified[i].g + VALUE_RAINBOW_FLOW) {
						ledModified[i].g += VALUE_RAINBOW_FLOW;
					} else if (rainbowColor[kIndex].g < ledModified[i].g - VALUE_RAINBOW_FLOW) {
						ledModified[i].g -= VALUE_RAINBOW_FLOW;
					}

					if (rainbowColor[kIndex].b > ledModified[i].b + VALUE_RAINBOW_FLOW) {
						ledModified[i].b += VALUE_RAINBOW_FLOW;
					} else if (rainbowColor[kIndex].b < ledModified[i].b - VALUE_RAINBOW_FLOW) {
						ledModified[i].b -= VALUE_RAINBOW_FLOW;
					}

					getMaxRgbValue(&ledModified[i]);

					i2cLength = i2cLength + 3;
				}

				_delayCount += VALUE_RAINBOW_FLOW;
				if (_delayCount >= 255 - VALUE_RAINBOW_FLOW) {					
					increaseRainbowIndex();
				}

				sendI2c();
			} else {	// fading
				arrived = 0;
				if (rainbowColor[_rainbowIndex].r > _currentLedAllColor.r) {
					++_currentLedAllColor.r;
				} else if (rainbowColor[_rainbowIndex].r < _currentLedAllColor.r) {
					--_currentLedAllColor.r;
				} else {
					++arrived;
				}
				if (rainbowColor[_rainbowIndex].g > _currentLedAllColor.g) {
					++_currentLedAllColor.g;
				} else if (rainbowColor[_rainbowIndex].g < _currentLedAllColor.g) {
					--_currentLedAllColor.g;
				} else {
					++arrived;
				}
				if (rainbowColor[_rainbowIndex].b > _currentLedAllColor.b) {
					++_currentLedAllColor.b;
				} else if (rainbowColor[_rainbowIndex].b < _currentLedAllColor.b) {
					--_currentLedAllColor.b;
				} else {
					++arrived;
				}
				setLed2All(_currentLedAllColor);

				if (arrived == 3) {
					if (++_rainbowIndex > 6) {
						_rainbowIndex = 0;
					}
				}
			}

		}
	}	// frame
}

static void fadeLED2(void){
		if(_pressed == PRESS_MODE_DOWN) return;
		_fadeLED2();
}

// 
static void fadeLED(void) {

    static int gFrame = 0;
	if(_fullLEDMode == 1) {
	    if(++gFrame == PWM_SPEED){
	        gFrame = 0;
            if(pwmDir==0)
                setPWM((uint8_t)(getBrightness(pwmCounter)));
            else if(pwmDir==2)
                setPWM((uint8_t)(getBrightness(PWM_MAX-pwmCounter)));
            else if(pwmDir==1 || pwmDir==3)
                pwmCounter++;

            // pwmDir 0~3 : idle
            if( pwmCounter>=PWM_MAX) {
                pwmCounter=0;
                pwmDir = (pwmDir+1)%4;
            }else{
                pwmCounter++;
            }
	    }

	}else if(_fullLEDMode == 3 || _fullLEDMode == 4){

		// 일정시간 유지
		if(downLevelStay > 0){
			downLevelStay--;
		}else{
			// 시간이 흐르면 레벨을 감소(_fullLEDMode == 3)/증가(_fullLEDMode == 4) 시킨다.
			if((_fullLEDMode == 3 && downLevelLife > 0) || (_fullLEDMode == 4 && downLevelLife < PWM_MAX)){
				pwmCounter++;
				if(pwmCounter >= PWM_SPEED){
					pwmCounter = 0;
					if(_fullLEDMode == 3){
						downLevelLife--;
					}else{
						downLevelLife++;
					}
					downLevel = downLevelMax - (PWM_MAX-downLevelLife) / (PWM_MAX/downLevelMax);
				}
			}else{
				if(_fullLEDMode == 3){
					downLevel = 0;
				}else{
					downLevel = downLevelMax;
				}
				pwmCounter = 0;
			}
		}
		setPWM((uint8_t)(getBrightness(downLevelLife)));

	}
}

//

void turnOnLedAll(void){
	_ledOff = false;

	setFullLedState();
	setLed2State();

}
void turnOffLedAll(void){
	_ledOff = true;

	setPWM(0);
	turnOffLED(LEDFULLLED);

	turnOffLed2();
}

void sleepLED(void){

	turnOffLED(LEDNUM);
	turnOffLED(LEDCAPS);
	turnOffLED(LEDSCROLL);

	setPWM(0);
	turnOffLED(LEDFULLLED);

	turnOffLed2();
}

void wakeUpLED(void){
	// DEBUG_PRINT(("wakeUpLED \n"));
	_ledOff = false;
	setLEDIndicate();

	setFullLedState();
	setLed2State();
}

void renderLED(void) {
	if(!ledInited || isSleep()){
		return;
	}

	blinkBootMapperLED();
	blinkIndicateLED();

	// LED 모드 저장.
	writeLEDMode();

	if(_ledOff) return;

	/* LED Fader */
	fadePWM();
}

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

	timer1PWMInit(8);

	if( _Led2Mode == 0 || (INTERFACE == INTERFACE_USB || INTERFACE == INTERFACE_USB_USER)){
		// led2가 off 상태이거나, usb 연결시에만 full led를 사용할 수 있음, 전류용량 때문.
		startFullLed();
	}else{
		stopFullLed();
	}


	initFullLEDStateAfter();

}


bool delegateFnControl(uint8_t xKeyidx, bool xIsExtraFnDown){
	uint8_t gModi = getModifierDownBuffer();
	if((xIsExtraFnDown && xKeyidx == LED_KEY)){
		if(gModi == 0x02 || gModi == 0x20){	// shift
			changeFullLedState(FULL_LED_MODE2);
		}else if(gModi == 0x01 || gModi == 0x10){	// control
			changeLed2KeyEventMode();
		}else{
			changeFullLedState(FULL_LED_MODE1);
		}
		return 0;
	}else if(xKeyidx == KEY_LED){
		changeFullLedState(FULL_LED_MODE1);
		return 0;
	}else if(xKeyidx == KEY_LED2){
		if(gModi == 0x01 || gModi == 0x10){	// control
			changeLed2KeyEventMode();
		}else{
			changeFullLedState(FULL_LED_MODE2);
		}
		return 0;
	}else if(xKeyidx == KEY_LED_UP){
		if(gModi == 0x02 || gModi == 0x20){
			increaseLedBrightness(FULL_LED_MODE2);
		}else{
			increaseLedBrightness(FULL_LED_MODE1);
		}
		return 0;
	}else if(xKeyidx == KEY_LED_DOWN){
		if(gModi == 0x02 || gModi == 0x20){
			reduceLedBrightness(FULL_LED_MODE2);
		}else{
			reduceLedBrightness(FULL_LED_MODE1);
		}
		return 0;
	}

	return 1;
}
