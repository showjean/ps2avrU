
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
#include "options.h"
#include "optionsled.h"
#include "keydownbuffer.h"
#include "hardwareinfo.h".h"

static lock_led_t lockLedStatus;

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
#define SAVE2_BIT_LED2_SKIP_FRAME   3
#define SAVE2_BIT_LOCK_LED_STATUS   4

static uint8_t _saved = 0;	//
static uint8_t _saved2 = 0;	//
//static bool confirmLed2Save = false;	//

static int downLevelStay = 0;
static uint8_t downLevel = 0;
static const uint8_t downLevelMax = 12;
static uint8_t downLevelLife = 0;

static uint8_t prevLEDstate = 0;
static uint8_t ledInited = 0;

static uint8_t _fullLEDMode = 0;	// 
static uint8_t _rgbMode = 0;	//
static uint8_t _rgbKeyEventMode = 0;	//
static uint8_t _rgbKeyEventType = 0;	//

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
#ifdef SPLIT
    #define TARGET_ADDR_SPLIT 0xB8
#endif

#define LED2_SKIP_FRAME_BASE   40
#define LED2_DELAY_MAX    250
#define LED2_DELAY_DEFAULT    120
static uint8_t led2Delay = LED2_DELAY_DEFAULT;
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
#define VALUE_OF_RAINBOW_FLOW_MAX 		20
#define VALUE_OF_RAINBOW_FLOW_MIN 		5
#define VALUE_OF_RAINBOW_FADE_MAX 50

static uint16_t _countOfFrame = 0;
static uint8_t numOfLeds = LED_NUM_DEFAULT; //LED_NUM_MAX;
//static cRGB_t led[LED_NUM_MAX];
static cRGB_t ledModified[LED_NUM_MAX];
static cRGB_t rainbowColor[LENGTH_OF_RAINBOW_COLOR];
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
static uint8_t _stepCount = 0;

static uint8_t _rainbowIndex = 0;

static void setLed2All(cRGB_t xRgb);
static void _setLed2All(cRGB_t *xRgb);
static void getMaxRgbValue(cRGB_t *xRgb);
static void turnOffLed2(void);
static void sendI2c(void);
static void setLedBalance(void);
static void setLed2BrightnessLimit(void);
static void setLed2BrightnessAfter(void);
static void changeRainbowColor(uint8_t *aTargetColor, uint8_t *aCurrentColor, uint8_t aValue);
static void setNextAvailableRainbowIndex(uint8_t *aIndex);
static void setModLength(uint8_t *aIndex);
static void applyStaticFullLed(void);
static void fadeRgbRainbowStatic(void);

static uint8_t LEDstate = 0;     ///< current state of the LEDs

#define blinkLedCountDelay 900

static uint8_t ledBlinkCount = 0;
static uint8_t targetLeds;
static uint8_t targetStatus;
/*
 * xPrev : LED의 이전 상태를 저장하고 있다. 이정 상태와 다를 경우에만 깜박이도록.
 */
void getLedBlink(uint8_t xLedPin, bool xMainLedOnOffStatus, bool xSubOnOffStatus, bool xSubPrevOnOffStatus, uint8_t *xCount){

    bool isChanged = false;

    if (xSubOnOffStatus && !xSubPrevOnOffStatus) { // light up

        *xCount =  4;   // off on off 1
        isChanged = true;

    } else if(!xSubOnOffStatus && xSubPrevOnOffStatus){

        *xCount =  2;   // off 1
        isChanged = true;

    }

    if(isChanged)
    {
        if(xMainLedOnOffStatus) targetStatus |= xLedPin;
        targetLeds |= xLedPin;
    }
}

void blinkIndicateLED(void) {
    static int counter = 0;
    const int countMAX = 100;
    //on off on off
    if(ledBlinkCount > 0){
        counter++;
        if(counter > countMAX){
            if(ledBlinkCount == 1 || ledBlinkCount == 3){
                if(targetLeds & LEDNUM) {
                    if(targetStatus & LEDNUM)
                    {
                        turnOnLED(LEDNUM);
                    }else{
                        turnOffLED(LEDNUM);
                    }
                }
                if(targetLeds & LEDCAPS) {
                    if(targetStatus & LEDCAPS)
                    {
                        turnOnLED(LEDCAPS);
                    }else{
                        turnOffLED(LEDCAPS);
                    }
                }
                if(targetLeds & LEDSCROLL) {
                    if(targetStatus & LEDSCROLL)
                    {
                        turnOnLED(LEDSCROLL);
                    }else{
                        turnOffLED(LEDSCROLL);
                    }
                }
            }else if(ledBlinkCount == 4 || ledBlinkCount == 2){
                if(targetLeds & LEDNUM) {
                    if(targetStatus & LEDNUM)
                    {
                        turnOffLED(LEDNUM);
                    }else{
                        turnOnLED(LEDNUM);
                    }
                }
                if(targetLeds & LEDCAPS) {
                    if(targetStatus & LEDCAPS)
                    {
                        turnOffLED(LEDCAPS);
                    }else{
                        turnOnLED(LEDCAPS);
                    }
                }
                if(targetLeds & LEDSCROLL) {
                    if(targetStatus & LEDSCROLL)
                    {
                        turnOffLED(LEDSCROLL);
                    }else{
                        turnOnLED(LEDSCROLL);
                    }
                }
            }
            counter = 0;

            ledBlinkCount--;
        }
    }else{
        targetStatus = 0;
        targetLeds = 0;
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

#define IS_LIGHT_UP_NL  lockLedStatus.nl == LOCK_LED_ALWAYS_ON \
                        || (lockLedStatus.nl == LOCK_LED_DEFAULT && (LEDstate & LED_STATE_NUM)) \
                        || (lockLedStatus.nl == LOCK_LED_REVERSE && !(LEDstate & LED_STATE_NUM)) \
                        || (lockLedStatus.nl == LOCK_LED_FN2_TOGGLE && getBeyondFN() == LAYER_FN2)\
                        || (lockLedStatus.nl == LOCK_LED_FN3_TOGGLE && getBeyondFN() == LAYER_FN3)
#define IS_LIGHT_UP_CL  lockLedStatus.cl == LOCK_LED_ALWAYS_ON \
                        || (lockLedStatus.cl == LOCK_LED_DEFAULT && (LEDstate & LED_STATE_CAPS)) \
                        || (lockLedStatus.cl == LOCK_LED_REVERSE && !(LEDstate & LED_STATE_CAPS)) \
                        || (lockLedStatus.cl == LOCK_LED_FN2_TOGGLE && getBeyondFN() == LAYER_FN2)\
                        || (lockLedStatus.cl == LOCK_LED_FN3_TOGGLE && getBeyondFN() == LAYER_FN3)
#define IS_LIGHT_UP_SL  lockLedStatus.sl == LOCK_LED_ALWAYS_ON \
                        || (lockLedStatus.sl == LOCK_LED_DEFAULT && (LEDstate & LED_STATE_SCROLL)) \
                        || (lockLedStatus.sl == LOCK_LED_REVERSE && !(LEDstate & LED_STATE_SCROLL)) \
                        || (lockLedStatus.sl == LOCK_LED_FN2_TOGGLE && getBeyondFN() == LAYER_FN2)\
                        || (lockLedStatus.sl == LOCK_LED_FN3_TOGGLE && getBeyondFN() == LAYER_FN3)

void blinkOnce(const int xStayMs){

    if ( IS_LIGHT_UP_NL ) { // light up num lock
        turnOffLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);    //
    }else{
        turnOnLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM); //
    }

    if ( IS_LIGHT_UP_CL ) { // light up caps lock
        turnOffLED(LEDCAPS); //PORTLEDS &= ~(1 << LEDCAPS); //
    } else {
        turnOnLED(LEDCAPS); //PORTLEDS |= (1 << LEDCAPS);   //
    }
    /*
        _delay_ms()에 xStayMs를 인자로 넣으면 hex 파일의 용량이 0x1000가량 증가한다.
        매뉴얼 펑션으로 _delay_ms(1)을 ms 만큼 루프시키도록 만들어서 사용;
    */
    __delay_ms(xStayMs);

    if ( IS_LIGHT_UP_NL ) { // light up num lock
        turnOnLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM); //
    }else{
        turnOffLED(LEDNUM);//PORTLEDS &= ~(1 << LEDNUM);    //
    }

    if ( IS_LIGHT_UP_CL ) { // light up caps lock
        turnOnLED(LEDCAPS); //PORTLEDS |= (1 << LEDCAPS);   //
    } else {
        turnOffLED(LEDCAPS); //PORTLEDS &= ~(1 << LEDCAPS); //
    }
}

void setLed(uint8_t xLed, bool xBool) {
    if (xBool) {
        if (xLed == LED_STATE_NUM) {
            turnOnLED(LEDNUM);
        } else if (xLed == LED_STATE_CAPS) {
            turnOnLED(LEDCAPS);
        }
        else if(xLed == LED_STATE_SCROLL) {
            turnOnLED(LEDSCROLL);
        }
    } else {
        if (xLed == LED_STATE_NUM) {
            turnOffLED(LEDNUM);
        } else if (xLed == LED_STATE_CAPS) {
            turnOffLED(LEDCAPS);
        }
        else if(xLed == LED_STATE_SCROLL) {
            turnOffLED(LEDSCROLL);
        }
    }
}

void initLED(void){
	// led pin
	DDRD |= (LEDNUM | LEDCAPS | LEDFULLLED | LEDSCROLL);	// output;
	PORTD &= ~(LEDNUM | LEDCAPS | LEDFULLLED | LEDSCROLL);	// low

	// initialize i2c function library
#if FIRMWARE == FIRMWARE_GB
    i2cInit();
    i2cSetBitrate(400);
#endif

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
//	DBG1(0xD4, (uchar *)&led2MaxSum400ma, 2);
//	DBG1(0xD2, (uchar *)&led2MaxSum200ma, 2);

    if(INTERFACE == INTERFACE_PS2){
    	led2SumLimit = led2MaxSum200ma;
    }else{
    	led2SumLimit = led2MaxSum400ma;
    }
    setLed2BrightnessLimit();
}

// options.h
// data = hid report
static uint8_t led2rainbowSettingCount = 0;

void getLedOptions(option_info_t *buffer){

	// report led2 info
	// num 1byte, mode 1byte, brightness 1byte, color1 3byte, color2 3byte, color3 3byte, rainbow colors 21byte, key color 3byte, fade mode 1byte
	buffer->num = numOfLeds;
	buffer->mode = _rgbMode;
	buffer->brightness = (uint8_t)(led2Brightness / 3);
	buffer->color1 = color1;
	buffer->color2 = color2;
	buffer->color3 = color3;
	uint8_t i;
	for(i=0;i<LENGTH_OF_RAINBOW_COLOR;++i){
		buffer->rainbow[i] = rainbowColor[i];
	}
	buffer->keymode = _rgbKeyEventMode;
	buffer->colorkey1 = color_key1;
	buffer->fadetype = _rainbowMode;

	//Ver1.1
	buffer->transitiondelay = led2Delay;

    buffer->fullledmode = _fullLEDMode;
    buffer->fullledbrightness = _ledBrightnessMax;

    // Ver 1.2
    buffer->lockled = lockLedStatus;
}

void __setLed2Mode(uint8_t xMode, uint8_t xKeyEventMode){
	_rgbMode = xMode;
	if(_rgbMode >= FULL_LED_MODE2_NUM) _rgbMode = 0;

	if(xKeyEventMode == 0 ){	// 키 이벤트가 없을 경우에만 모드 변경 적용;
		_pressed = PRESS_MODE_UP;
		setLed2State();
	}else{
		_pressed = PRESS_MODE_CHANGE;	// up event로 led 컨트롤을 미룬다.
	}

	_saved |= BV(SAVE_BIT_LED_MODE);
}
void setLedOptions(uint8_t *data){
	/*
	 * data[0] = report id;
	 * data[1] = index
	 * data[2] ~ = data...
	 */
	 if(*(data+1) == OPTION_INDEX_COLOR_RAINBOW_INIT)
	 {
		led2rainbowSettingCount = 0;
	 }
	 else if(*(data+1) == OPTION_INDEX_COLOR_RAINBOW)
	 {
	 	if(led2rainbowSettingCount > 0 && led2rainbowSettingCount < 4){
			rainbowColor[(led2rainbowSettingCount*2)-2] = *((cRGB_t *)(data+2));
			rainbowColor[(led2rainbowSettingCount*2)-1] = *((cRGB_t *)(data+5));
		}else if(led2rainbowSettingCount==4){
			rainbowColor[6] = *((cRGB_t *)(data+2));

			_saved |= BV(SAVE_BIT_LED2_COLOR_RAINBOW);			
		}
		++led2rainbowSettingCount;
	 }
	 else if(*(data+1) == OPTION_INDEX_COLOR_SET1)
	 {
	 	color1 = *((cRGB_t *)(data+2));
		_saved |= BV(SAVE_BIT_LED2_COLOR1);

	    setLed2State();
	 }
	 else if(*(data+1) == OPTION_INDEX_COLOR_SET2)
	 {
	 	color2 = *((cRGB_t *)(data+2));
		_saved |= BV(SAVE_BIT_LED2_COLOR2);

	    setLed2State();
	 }
	 else if(*(data+1) == OPTION_INDEX_COLOR_SET3)
	 {
	 	color3 = *((cRGB_t *)(data+2));
		_saved |= BV(SAVE_BIT_LED2_COLOR3);

	    setLed2State();
	 }
	 else if(*(data+1) == OPTION_INDEX_COLOR_KEY_SET1)
	 {
	 	color_key1 = *((cRGB_t *)(data+2));
		_saved2 |= BV(SAVE2_BIT_LED2_COLOR_KEY1);
	 }
	 else if(*(data+1) == OPTION_INDEX_MODE)
	 {
	 	__setLed2Mode(*(data+2), 0);	// 항상 led 재설정;
	 }
	 else if(*(data+1) == OPTION_INDEX_LED_BRIGHTNESS)
	 {
	 	led2Brightness = *(data+2) * 3;
		if(led2Brightness == 0) led2Brightness = 3;
		setLed2BrightnessLimit();

		setLed2BrightnessAfter();
		setLed2State();
	 }
	 else if(*(data+1) == OPTION_INDEX_MODE_KEY)
	 {
	 	_rgbKeyEventMode = *(data+2);
		_saved2 |= BV(SAVE2_BIT_LED2_KEY_EVENT);	
	 }
	 else if(*(data+1) == OPTION_INDEX_MODE_FADE_TYPE)
	 {
	 	_rainbowMode = *(data+2);
		_delayCount = 0;
		_changeCount = 0;
		_rainbowIndex = 0;
		_stepCount = 0;
		_saved2 |= BV(SAVE2_BIT_LED2_FADE_TYPE);
	 }
	 else if(*(data+1) == OPTION_INDEX_LED_NUM)
	 {
	 	turnOffLed2();
		setLed2Num(*(data+2));
	    setLedBalance();

		_saved |= BV(SAVE_BIT_LED2_NUM);
	 }
     else if(*(data+1) == OPTION_INDEX_FULL_LED_MODE)
     {
         _fullLEDMode = *(data+2);
         _saved |= BV(SAVE_BIT_LED_MODE);
         setFullLedState();
     }
     else if(*(data+1) == OPTION_INDEX_FULL_LED_BRIGHTNESS)
     {
         _ledBrightnessMax = *(data+2);

         applyStaticFullLed();
         _saved |= BV(SAVE_BIT_LED_BRITNESS_MAX);
     }
     else if(*(data+1) == OPTION_INDEX_TRANSITION_DELAY)
     {
         led2Delay = *(data+2);

         _saved2 |= BV(SAVE2_BIT_LED2_SKIP_FRAME);
     }
     else if(*(data+1) == OPTION_INDEX_LOCK_LED_NL)
     {
         lockLedStatus.nl = *(data+2);
//         prevLEDstate ^= LED_STATE_NUM;
         setLEDIndicate();

         _saved2 |= BV(SAVE2_BIT_LOCK_LED_STATUS);
     }
     else if(*(data+1) == OPTION_INDEX_LOCK_LED_CL)
     {
         lockLedStatus.cl = *(data+2);
//         prevLEDstate ^= LED_STATE_CAPS;
         setLEDIndicate();

         _saved2 |= BV(SAVE2_BIT_LOCK_LED_STATUS);
     }
     else if(*(data+1) == OPTION_INDEX_LOCK_LED_SL)
     {
         lockLedStatus.sl = *(data+2);
//         prevLEDstate ^= LED_STATE_SCROLL;
         setLEDIndicate();

         _saved2 |= BV(SAVE2_BIT_LOCK_LED_STATUS);
     }

	ledStateCount = 0;
}

static void changeLed2KeyEventMode(void){
//	_Led2KeyEventMode = (_Led2KeyEventMode + 1);
	if(++_rgbKeyEventMode == FULL_LED_MODE2_KEY_EVENT_NUM) _rgbKeyEventMode = 0;
	_saved2 |= BV(SAVE2_BIT_LED2_KEY_EVENT);
}

void stopPwmLed(bool xIsStop){
    // stop timer for usb report

    if(xIsStop)
    {
        timer1SetPrescaler( TIMERRTC_CLK_STOP );	// set prescaler
        cbi(TIMSK, TOIE1);
        setPWM(0);
        turnOffLED(LEDFULLLED);
        timer1PWMBOff();
    }
    else
    {
        timer1Init();
        timer1PWMBOn();
        if(_fullLEDMode == 2){
            setFullLedState();
        }
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
	_rgbMode = (eeprom_read_byte((uint8_t *)EEPROM_LED_MODE) >> 4) & 0x0F;
	if(_rgbMode == 0x0F){
		_rgbMode = 0;
	}

	// led2 color1~3
	eeprom_read_block(&color1, (uint8_t *)EEPROM_LED2_COLOR_1, 3);
	eeprom_read_block(&color2, (uint8_t *)EEPROM_LED2_COLOR_2, 3);
	eeprom_read_block(&color3, (uint8_t *)EEPROM_LED2_COLOR_3, 3);

	// set rainbow color

	// led2 rainbow
	eeprom_read_block(&rainbowColor[0], (uint8_t *)EEPROM_LED2_COLOR_RAINBOW, 21 /* LENGTH_OF_RAINBOW_COLOR * 3 */);

	// led2 rainbow fading type
	_rainbowMode = eeprom_read_byte((uint8_t *)EEPROM_LED2_FADE_TYPE);

	// led2 num
	led2Brightness = eeprom_read_byte((uint8_t *)EEPROM_LED2_BRIGHTNESS);
	led2Brightness = (led2Brightness == 0xFF ? 0x4F : led2Brightness) * 3;
	setLed2Num(eeprom_read_byte((uint8_t *)EEPROM_LED2_NUM));

	// led2 key mode
	_rgbKeyEventMode = eeprom_read_byte((uint8_t *)EEPROM_LED2_KEY_EVENT) & 0x0F;	// 0xF0 : type, 0x0F : mode
	if(_rgbKeyEventMode == 0x0F){
		_rgbKeyEventMode = 0;
	}
	// led2 key color 1
	eeprom_read_block(&color_key1, (uint8_t *)EEPROM_LED2_COLOR_KEY1, 3);

	led2Delay = eeprom_read_byte((uint8_t *)EEPROM_LED2_SKIP_FRAME);
	if(led2Delay == 0xFF)
    {
	    led2Delay = LED2_DELAY_DEFAULT;
    }
	else if(led2Delay > LED2_DELAY_MAX)
    {
	    led2Delay = LED2_DELAY_MAX;
    }

	// lock led status
	eeprom_read_block(&lockLedStatus, (uint8_t *)EEPROM_LOCK_LED_STATUS, 3);
	if(lockLedStatus.nl == 0xFF) lockLedStatus.nl = LOCK_LED_DEFAULT;
	if(lockLedStatus.cl == 0xFF) lockLedStatus.cl = LOCK_LED_DEFAULT;
	if(lockLedStatus.sl == 0xFF) lockLedStatus.sl = LOCK_LED_DEFAULT;

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
	if(_fullLEDMode == 1)
	{
		pwmCounter = 0;
		pwmDir = 0;
	}
	else if(_fullLEDMode == 2)
	{
		setPWM(getBrightness(PWM_MAX));
	}
	else if(_fullLEDMode == 3 || _fullLEDMode == 4)
	{
		downLevel = 3;
		if(_fullLEDMode == 4) downLevel = 1; 	// 3으로 시작하면 효과가 별로 안 남.
		downLevelLife = downLevel * PWM_MAX / downLevelMax;
	}
	else
	{
		setPWM(0);
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

	// if(_Led2Mode == 2 || _Led2Mode == 3 || _Led2Mode == 4){
	if(_rgbMode > 1 && _rgbMode < 5){
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
		__setLed2Mode(_rgbMode+1, _rgbKeyEventMode);	// 키보드를 이용할 경우 키이벤트에 따라 재설정 판가름;
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

static uint8_t updateLed(uint8_t xLockStatus, uint8_t xLedPin, uint8_t xLedState)
{
    uint8_t gCount = 0;

    if(xLockStatus == LOCK_LED_ALWAYS_ON)
    {
        turnOnLED(xLedPin);
        getLedBlink(xLedPin, true, (LEDstate & xLedState), (prevLEDstate & xLedState), &gCount);
        if(gCount == 0) getLedBlink(xLedPin, true, getBeyondFN(), getBeyondFNPrev(), &gCount);
    }
    else if(xLockStatus == LOCK_LED_ALWAYS_OFF)
    {
        turnOffLED(xLedPin);
        getLedBlink(xLedPin, false, (LEDstate & xLedState), (prevLEDstate & xLedState), &gCount);
        if(gCount == 0) getLedBlink(xLedPin, false, getBeyondFN(), getBeyondFNPrev(), &gCount);
    }
    else if(xLockStatus == LOCK_LED_DEFAULT)
    {
        if (LEDstate & xLedState) {
            turnOnLED(xLedPin);
        } else {
            turnOffLED(xLedPin);
        }
        getLedBlink(xLedPin, (LEDstate & xLedState), getBeyondFN(), getBeyondFNPrev(), &gCount);
    }
    else if(xLockStatus == LOCK_LED_REVERSE)
    {
        if (LEDstate & xLedState) {
            turnOffLED(xLedPin);
        } else {
            turnOnLED(xLedPin);
        }
        getLedBlink(xLedPin, !(LEDstate & xLedState), getBeyondFN(), getBeyondFNPrev(), &gCount);
    }
    else if(xLockStatus == LOCK_LED_FN2_TOGGLE)
    {
        if (getBeyondFN() == LAYER_FN2) {
            turnOnLED(xLedPin);
        } else {
            turnOffLED(xLedPin);
        }
        getLedBlink(xLedPin, (getBeyondFN() == LAYER_FN2), (LEDstate & xLedState), (prevLEDstate & xLedState), &gCount);
    }
    else if(xLockStatus == LOCK_LED_FN3_TOGGLE)
    {
        if (getBeyondFN() == LAYER_FN3) {
            turnOnLED(xLedPin);
        } else {
            turnOffLED(xLedPin);
        }
        getLedBlink(xLedPin, (getBeyondFN() == LAYER_FN3), (LEDstate & xLedState), (prevLEDstate & xLedState), &gCount);
    }
    else if(xLockStatus == LOCK_LED_FN23_TOGGLE)
    {
        if (getBeyondFN()) {
            turnOnLED(xLedPin);
        } else {
            turnOffLED(xLedPin);
        }
        getLedBlink(xLedPin, getBeyondFN(), (LEDstate & xLedState), (prevLEDstate & xLedState), &gCount);
    }

    return gCount;
}
// TODO
/**
 * 어떤 변화라도 on/off 깜박이는 것은 동일 하므로, NCS별로 변화가 있었음을 표시해주면 해당 LED를 깜박이는 걸로;
 */
void setLEDIndicate(void) {

    // nl led를 다른 용도로 사용 할 경우 깜박임으로 indicating

    uint8_t gCount = 0;
    gCount = updateLed(lockLedStatus.nl, LEDNUM, LED_STATE_NUM);
    if(gCount > 0) ledBlinkCount = gCount;
    gCount = updateLed(lockLedStatus.cl, LEDCAPS, LED_STATE_CAPS);
    if(gCount > 0) ledBlinkCount = gCount;
    gCount = updateLed(lockLedStatus.sl, LEDSCROLL, LED_STATE_SCROLL);
    if(gCount > 0) ledBlinkCount = gCount;

	prevLEDstate = LEDstate;

}

static void writeLEDMode(void) {
	// LED 모드 저장, 너무많은 eeprom접근을 막기위해 일정 간격으로 저장한다.
//	const int countMAX = 1000;
	if((_saved > 0 || _saved2 > 0) && ++ledStateCount == 3000){
		ledStateCount = 0;

		// full led
		if(((_saved >> SAVE_BIT_LED_MODE) & 0x01)){	// || ((_saved >> SAVE_BIT_LED2_MODE) & 0x01)){
			eeprom_update_byte((uint8_t *)EEPROM_LED_MODE, (_fullLEDMode & 0x0F) | ((_rgbMode & 0x0F) << 4));
		}

		// brightness
		if(((_saved >> SAVE_BIT_LED_BRITNESS_MAX) & 0x01)){
			eeprom_update_byte((uint8_t *)EEPROM_LED_BRIGHTNESS, _ledBrightnessMax);
		}
		if(((_saved >> SAVE_BIT_LED2_BRITNESS_MAX) & 0x01)){
			eeprom_update_byte((uint8_t *)EEPROM_LED2_BRIGHTNESS, led2Brightness/3);
		}

		// led num SAVE_BIT_LED2_NUM
		if(((_saved >> SAVE_BIT_LED2_NUM) & 0x01)){
			setFullLedState();
		    setLed2State();

			eeprom_update_byte((uint8_t *)EEPROM_LED2_NUM, numOfLeds);
		}

		// led2 color1~3
		if(((_saved >> SAVE_BIT_LED2_COLOR1) & 0x01)){
			eeprom_update_block(&color1, (uint8_t *)(EEPROM_LED2_COLOR_1), 3);
		}
		if(((_saved >> SAVE_BIT_LED2_COLOR2) & 0x01)){
			eeprom_update_block(&color2, (uint8_t *)(EEPROM_LED2_COLOR_2), 3);
		}
		if(((_saved >> SAVE_BIT_LED2_COLOR3) & 0x01)){
			eeprom_update_block(&color3, (uint8_t *)(EEPROM_LED2_COLOR_3), 3);
		}

		// led2 rainbow
		if(((_saved >> SAVE_BIT_LED2_COLOR_RAINBOW) & 0x01)){

			eeprom_update_block(&rainbowColor, (uint8_t *)(EEPROM_LED2_COLOR_RAINBOW), 21 /* LENGTH_OF_RAINBOW_COLOR * 3 */);

		}

		// led2 rainbow fading type
		if(((_saved2 >> SAVE2_BIT_LED2_FADE_TYPE) & 0x01)){
			eeprom_update_byte((uint8_t *)EEPROM_LED2_FADE_TYPE, _rainbowMode);
		}

		// led2 key color1
		if(((_saved2 >> SAVE2_BIT_LED2_KEY_EVENT) & 0x01)){
			eeprom_update_byte((uint8_t *)EEPROM_LED2_KEY_EVENT, (_rgbKeyEventMode & 0x0F) | ((_rgbKeyEventType & 0x0F) << 4));
		}

		if(((_saved2 >> SAVE2_BIT_LED2_COLOR_KEY1) & 0x01)){
			eeprom_update_block(&color_key1, (uint8_t *)(EEPROM_LED2_COLOR_KEY1), 3);
		}

        if(((_saved2 >> SAVE2_BIT_LED2_SKIP_FRAME) & 0x01)){
            eeprom_update_byte((uint8_t *)EEPROM_LED2_SKIP_FRAME, led2Delay);
        }

        if(((_saved2 >> SAVE2_BIT_LOCK_LED_STATUS) & 0x01)){
            eeprom_update_block(&lockLedStatus, (uint8_t *)(EEPROM_LOCK_LED_STATUS), 3);
        }

		_saved = 0;
		_saved2 = 0;

		blinkOnce(50);
	}
}

static uint16_t getBrightness(uint16_t xValue){
	uint16_t gBri = xValue * _ledBrightnessMax / PWM_MAX;
	if(gBri > ledBrightnessLimit)
	{
		gBri = ledBrightnessLimit;
	}
	// gBri = gBri > ledBrightnessLimit ? ledBrightnessLimit : gBri;

	return gBri;
}

static void fadePWM(void){
	fadeLED();
	fadeLED2();
}

/*
 * 3개의 LED를 사용하기 때문에 3개 모두의 값이 제한 값이 초과 되지 않도록 하고,
 * 2개 이하의 LED를 이용할 경우 개별 값은 LED2_COLOR_MAX까지 설정할 수 있도록 하여 밝기를 확보한다.
 */
static void getMaxRgbValue(cRGB_t *xRgb){
	uint16_t gSum = xRgb->r + xRgb->g + xRgb->b;
	if(gSum > led2Brightness)
	{		
		xRgb->r = (uint32_t)(xRgb->r) * led2Brightness / gSum;
		xRgb->g = (uint32_t)(xRgb->g) * led2Brightness / gSum;
		xRgb->b = (uint32_t)(xRgb->b) * led2Brightness / gSum;
	}

	// 0xffffff는 간혹 꺼지는 LED가 있어 이를 패치
    if(xRgb->r > 2 && xRgb->r == xRgb->g && xRgb->r == xRgb->b)
    {
        xRgb->g -= 1;
        xRgb->b -= 2;
    }

}

static void turnOffLed2(void){
	ledModified[0].r = 0;
	ledModified[0].g = 0;
	ledModified[0].b = 0;	
	prevRgb.r = 0; // = (cRGB_t){0,0,0};
	prevRgb.g = 0;
	prevRgb.b = 0;
	// _currentLedAllColor = ledModified[0];
	_currentLedAllColor.r = 0; 
	_currentLedAllColor.g = 0;
	_currentLedAllColor.b = 0;
	_setLed2All(&_currentLedAllColor);
}


void applyKeyDownForFullLED(uint8_t keyidx, uint8_t col, uint8_t row, uint8_t isDown){

	if(isSleep() || _ledOff){
		return;
	}

	if(isDown){
		// full led
		if(_fullLEDMode > 2){
			// 키를 누르면 값을 증가 시킨다.
			downLevelStay = 500; //511;
			if(downLevel < downLevelMax && _fullLEDMode == 3)
			{
				downLevel++;
			}
			else if(downLevel > 0 && _fullLEDMode == 4)
			{
				downLevel--;
			}

			downLevelLife = (uint16_t)(downLevel * PWM_MAX) / downLevelMax;
		}

		// led2
		if(_rgbMode > 0 && keyidx != KEY_LED_ON_OFF){
			if(_rgbKeyEventMode == 1)
			{		// color 1
				ledModified[0] = color_key1;
				getMaxRgbValue(&ledModified[0]);
				prevRgb = ledModified[0];
				_setLed2All(&ledModified[0]);

				_pressed = PRESS_MODE_DOWN;
			}
			else if(_rgbKeyEventMode == 2)
			{	// complementary color
				ledModified[0].g = 255 - _currentLedAllColor.g;
				ledModified[0].r = 255 - _currentLedAllColor.r;
				ledModified[0].b = 255 - _currentLedAllColor.b;
				getMaxRgbValue(&ledModified[0]);
                prevRgb = ledModified[0];
                _setLed2All(&ledModified[0]);

				_pressed = PRESS_MODE_DOWN;
			}
		}
	}else{	// up
		// led2
		if(_pressed == PRESS_MODE_DOWN){
		    if (_rgbMode == 1) { // 1 = rainbow
                if (_rainbowMode == 3)  //static
                {
                    fadeRgbRainbowStatic();

                }else if(_rainbowMode == 4) { // flow

                } else {    // others
                    setLed2All(_currentLedAllColor);
                }

            }
		    else
		    {
		        setLed2All(_currentLedAllColor);
		    }
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
//	DBG1(0xAE, (uchar *)&_rgbMode, 1);

	if((INTERFACE == INTERFACE_PS2)){
		if(_rgbMode == 0)
		    stopPwmLed(false);
		else
		    stopPwmLed(true);
    }

    
	if(_rgbMode == 1)
	{
		setLedBalance();
		_delayCount = 0;
		_changeCount = 0;
		_stepCount = 0;
		
	}
	else if(_rgbMode == 2)
	{
		setLedBalance();
		_currentLedAllColor = color1;
		setLed2All(_currentLedAllColor);
		
	}
	else if(_rgbMode == 3)
	{
		setLedBalance();
		_currentLedAllColor = color2;
		setLed2All(_currentLedAllColor);
		
	}
	else if(_rgbMode == 4)
	{
		setLedBalance();
		_currentLedAllColor = color3;
		setLed2All(_currentLedAllColor);		
	}
	else
	{
		
		turnOffLed2();
		ledBrightnessLimit = 255;
	
	}
	
	applyStaticFullLed();

}

static void sendI2c(void){

#if FIRMWARE == FIRMWARE_GB
    i2cLength = numOfLeds * 3;
#ifdef SPLIT
    /**
     *
     */
    uint8_t gLengthHalf = i2cLength >> 1;
    i2cLength = i2cLength - gLengthHalf;
    i2cMasterSendNI(TARGET_ADDR, i2cLength, (u08 *)ledModified);
    i2cMasterSendNI(TARGET_ADDR_SPLIT, gLengthHalf, (u08 *)ledModified+i2cLength);
#else
    i2cMasterSendNI(TARGET_ADDR, i2cLength, (u08 *)ledModified);
	
#endif
    i2cLength = 0;
#endif

}

static void _setLed2All(cRGB_t *xRgb){
	uint8_t i;

	for (i=0; i<numOfLeds; ++i){
		ledModified[i] = *xRgb;
	}

	i2cLength = numOfLeds * 3;

	sendI2c();

}


static void increaseRainbowIndex(void)
{
	if (++_rainbowIndex >= LENGTH_OF_RAINBOW_COLOR) {
		_rainbowIndex = 0;
	}
}
static void setModLength(uint8_t *aIndex)
{
	if(*aIndex >= LENGTH_OF_RAINBOW_COLOR)
	{
		*aIndex -= LENGTH_OF_RAINBOW_COLOR;
	}
}

/**
 * 0x000000 컬러는 flow, static 대상에서 제외;
 */
static void setNextAvailableRainbowIndex(uint8_t *aIndex)
{
	uint8_t i;

    setModLength(aIndex);

	for (i = 0; i < numOfLeds; ++i) {

		if(rainbowColor[*aIndex].r == 0
			&& rainbowColor[*aIndex].g == 0
			&& rainbowColor[*aIndex].b == 0)
		{
			*aIndex	+= 1;

			setModLength(aIndex);
		}
		else
		{
			break;
		}
	}
}

static void setLed2All(cRGB_t xRgb){

//    DBG1(0x41, (uchar *)&xRgb, 3);

	getMaxRgbValue(&xRgb);
//DBG1(0x44, (uchar *)&xRgb, 3);

	if(prevRgb.r == xRgb.r && prevRgb.g == xRgb.g && prevRgb.b == xRgb.b) return;


	prevRgb = xRgb;
//DBG1(0x45, (uchar *)&xRgb, 3);

	_setLed2All(&xRgb);
}

static void changeRainbowColor(uint8_t *aTargetColor, uint8_t *aCurrentColor, uint8_t aValue)
{
	if (*aTargetColor > *aCurrentColor + aValue) {
		*aCurrentColor += aValue;
	} else if (*aTargetColor < *aCurrentColor - aValue) {
		*aCurrentColor -= aValue;
	}
	else
	{
	    *aCurrentColor = *aTargetColor;
	}
}

static void fadeRgbRainbowStatic(void)
{
    _changeCount = 0;
    uint8_t i;
    for (i = 0; i < numOfLeds; i++) {
        setNextAvailableRainbowIndex(&_changeCount);
        ledModified[i] = rainbowColor[_changeCount];

        getMaxRgbValue(&ledModified[i]);

        _changeCount++;
    }
    _currentLedAllColor = ledModified[i];

    sendI2c();
}

static void __fadeLED2(void){

	if(++_countOfFrame >= LED2_SKIP_FRAME_BASE){	// setDelay(led2TickPerFrame)){
		_countOfFrame = 0;
		uint8_t i;
		uint16_t gValue;
		if (_rgbMode == 1) { // 1 = rainbow

			if (_rainbowMode == 1) {	// immediately
				if (_delayCount++ >= led2Delay) {
				    _delayCount = 0;
					_currentLedAllColor = rainbowColor[_rainbowIndex];
					setLed2All(_currentLedAllColor);
					increaseRainbowIndex();
				}
			} else if (_rainbowMode == 2) {	// sequential
                    if (_changeCount < numOfLeds) {
                        for (i = 0; i < numOfLeds; ++i) {
                            if (i > _changeCount) {
                                // old color
                                ledModified[i] = _currentLedAllColor;
                            } else {
                                // new color
                                ledModified[i] = rainbowColor[_rainbowIndex];
                                getMaxRgbValue(&ledModified[i]);
                            }

                        }
                        ++_changeCount;

                        sendI2c();
                    }

                    if(_changeCount == numOfLeds)
                    {
                        ++_changeCount;
                        _currentLedAllColor = rainbowColor[_rainbowIndex];
                        getMaxRgbValue(&_currentLedAllColor);
                    }

                    if (_changeCount > numOfLeds && ++_stepCount >= led2Delay) {
                        _stepCount = 0;
                        _changeCount = 0;
                        increaseRainbowIndex();
                    }
			}else  if (_rainbowMode == 3) {	// static
				if (_delayCount++ == 0) {
					fadeRgbRainbowStatic();
				}
			} else if (_rainbowMode == 4) {	// flow

			    // TODO
			    /**
			     * 굼뱅이처럼 변환되는 부분을 해결하고 싶다.
			     */
                _changeCount = _rainbowIndex;
                gValue = (VALUE_OF_RAINBOW_FLOW_MAX-VALUE_OF_RAINBOW_FLOW_MIN) - ((VALUE_OF_RAINBOW_FLOW_MAX-VALUE_OF_RAINBOW_FLOW_MIN) * led2Delay / LED2_DELAY_MAX) + VALUE_OF_RAINBOW_FLOW_MIN;
                for (i = 0; i < numOfLeds; ++i) {
                    setNextAvailableRainbowIndex(&_changeCount);

                    changeRainbowColor(&rainbowColor[_changeCount].r, &ledModified[i].r, gValue);
                    changeRainbowColor(&rainbowColor[_changeCount].g, &ledModified[i].g, gValue);
                    changeRainbowColor(&rainbowColor[_changeCount].b, &ledModified[i].b, gValue);

                    getMaxRgbValue(&ledModified[i]);

                    _changeCount++;
                }

                _currentLedAllColor = ledModified[i];

                sendI2c();

                _stepCount += gValue;
                if (_stepCount >= 255 - gValue) {
                    _stepCount = 0;

//                    increaseRainbowIndex();
                    _rainbowIndex++;
                    setNextAvailableRainbowIndex(&_rainbowIndex);

                }


			} else {	// fading
			    // led2Delay : 1~ 250
			    // variation : 1 ~ 50
			    gValue = VALUE_OF_RAINBOW_FADE_MAX - (VALUE_OF_RAINBOW_FADE_MAX * led2Delay / LED2_DELAY_MAX) + 1;
                changeRainbowColor(&rainbowColor[_rainbowIndex].r, &_currentLedAllColor.r, (uint8_t)gValue);
                changeRainbowColor(&rainbowColor[_rainbowIndex].g, &_currentLedAllColor.g, (uint8_t)gValue);
                changeRainbowColor(&rainbowColor[_rainbowIndex].b, &_currentLedAllColor.b, (uint8_t)gValue);

                setLed2All(_currentLedAllColor);

                //if (++_stepCount >= 255) {
                if(rainbowColor[_rainbowIndex].r == _currentLedAllColor.r
                        && rainbowColor[_rainbowIndex].g == _currentLedAllColor.g
                        && rainbowColor[_rainbowIndex].b == _currentLedAllColor.b)
                {
                    increaseRainbowIndex();
                }

			}

		}
	}	// frame
}

static void fadeLED2(void){
		if(_pressed == PRESS_MODE_DOWN) return;
		__fadeLED2();
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
                //pwmDir = (pwmDir+1)%4;
                if(++pwmDir >= 4) pwmDir = 0;
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
	_ledOff = false;
	setLEDIndicate();

	setFullLedState();
	setLed2State();
}

void renderLED(void) {
    static int _sleepOffCount = 0;
	if(!ledInited || isSleep()){
	    if(isSleep() && _sleepOffCount++ > 10000)
	    {
	        _sleepOffCount = 0;
	        sleepLED();
	    }
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

	if( _rgbMode == 0 || (INTERFACE == INTERFACE_USB)){
		// led2가 off 상태이거나, usb 연결시에만 full led를 사용할 수 있음, 전류용량 때문.
	    stopPwmLed(false);
	}else{
	    stopPwmLed(true);
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
