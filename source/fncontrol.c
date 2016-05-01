
#include "timerinclude.h"
// #include "print.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include <util/delay.h>
#include <string.h>

#include "fncontrol.h"
#include "keymap.h"
#include "keymatrix.h"
#include "ledrender.h"
#include "keymapper.h"
#include "dualaction.h"
#include "keymatrix.h"
//#include "keydownbuffer.h"
#include "ps2avru_util.h"
#include "oddebug.h"

static uint8_t _beyondFnLed;	// 0: off, 1:NL, 2:SL
// for KEY_BEYOND_FN;
static uint8_t _beyondFnIndex = 0;     //KEY_BEYOND_FN state
static uint8_t _prevBeyondFnIndex = 0;
static bool _isDownExtraFN = false;
static bool _isDownQuickMacro = false;
static bool _isReadyQuickMacro = false;
static uint8_t _quickMacroIndex = 255;
static uint8_t _isLockKey = LOCK_NOT_SET;
static uint8_t _isLockWin = LOCK_NOT_SET;

static bool _ledOff = false;

bool isDownExtraFn(void)
{
    return _isDownExtraFN;
}

uint8_t getBeyondFN(void){
    return _beyondFnIndex;
}
uint8_t getBeyondFNPrev(void){
    return _prevBeyondFnIndex;
}

uint8_t getBeyondFnLed(void){
    return _beyondFnLed;
}

#ifdef FN_TOGGLE_TEST
static void __setBeyondFnLed(void)
{
    setLEDIndicate();

    if(getBeyondFnLed() == BEYOND_FN_LED_NL){
        setLed(LED_STATE_NUM, getBeyondFN());
    }else if(getBeyondFnLed() == BEYOND_FN_LED_SL){
        setLed(LED_STATE_SCROLL, getBeyondFN());
    }
}

void setBeyondFnLed(uint8_t xLed){
    _beyondFnLed = xLed;
    /*
    	 TOGGLE_BEYOND_FN_LED_NL == 1, TOGGLE_BEYOND_FN_LED_SL == 1 : off
    	 TOGGLE_BEYOND_FN_LED_NL == 0, TOGGLE_BEYOND_FN_LED_SL == 1 : NL
    	 TOGGLE_BEYOND_FN_LED_NL == 1, TOGGLE_BEYOND_FN_LED_SL == 0 : SL
    	 */

    if(_beyondFnLed == BEYOND_FN_LED_NL){
    	eeprom_update_byte((uint8_t *) EEPROM_ENABLED_OPTION, ((eeprom_read_byte((uint8_t *) EEPROM_ENABLED_OPTION) & ~(_BV(TOGGLE_BEYOND_FN_LED_NL))) | (_BV(TOGGLE_BEYOND_FN_LED_SL))));
    }else if(_beyondFnLed == BEYOND_FN_LED_SL){
        eeprom_update_byte((uint8_t *) EEPROM_ENABLED_OPTION, ((eeprom_read_byte((uint8_t *) EEPROM_ENABLED_OPTION) | (_BV(TOGGLE_BEYOND_FN_LED_NL))) & ~(_BV(TOGGLE_BEYOND_FN_LED_SL))));
    }else{
        eeprom_update_byte((uint8_t *) EEPROM_ENABLED_OPTION, ((eeprom_read_byte((uint8_t *) EEPROM_ENABLED_OPTION) | (_BV(TOGGLE_BEYOND_FN_LED_NL))) | (_BV(TOGGLE_BEYOND_FN_LED_SL))));
    }

#ifdef FN_TOGGLE_TEST
    __setBeyondFnLed();
#endif
}
#endif

#if 0 //ndef DISABLE_HARDWARE_MENU

#define CMD_OFF_BEYOND_FN_LED 1
#define CMD_NL_BEYOND_FN_LED 2
#define CMD_SL_BEYOND_FN_LED 3
#define CMD_EXIT_BEYOND_FN_LED 4
#define CMD_BACK_BEYOND_FN_LED 6

const char str_select_beyond_fn[] PROGMEM = "fn2/3 led";
const char str_beyond_fn_led_nl[] PROGMEM = "nl";
const char str_beyond_fn_led_sl[] PROGMEM = "sl";

void printMenuBeyondFn(void);
void printContentsBeyondFn(void);
void putKeyindexBeyondFn(uint8_t xCmd, uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, uint8_t xIsDown);

keymapper_driver_t driverKeymapperBeyondFn = {
    printMenuBeyondFn,
    printContentsBeyondFn,
    putKeyindexBeyondFn
};

static void printBeyondFnLedState(void){
    printStringFromFlash(str_select_beyond_fn);
    printStringFromFlash(str_space);
    printStringFromFlash(str_colon);
    printStringFromFlash(str_space);
    if(getBeyondFnLed() == BEYOND_FN_LED_NL){
        printStringFromFlash(str_beyond_fn_led_nl);
    }else if(getBeyondFnLed() == BEYOND_FN_LED_SL){
        printStringFromFlash(str_beyond_fn_led_sl);
    }else{
        printStringFromFlash(str_off);
    }
}

void printMenuBeyondFn(void){
    printBeyondFnLedState();
}

void printContentsBeyondFn(void){
    printBeyondFnLedState();
    printEnter();
    printString(toString(CMD_OFF_BEYOND_FN_LED));
    printStringFromFlash(str_colon);
    printStringFromFlash(str_off);
    printEnter();
    printString(toString(CMD_NL_BEYOND_FN_LED));
    printStringFromFlash(str_colon);
    printStringFromFlash(str_beyond_fn_led_nl);
    printEnter();
    printString(toString(CMD_SL_BEYOND_FN_LED));
    printStringFromFlash(str_colon);
    printStringFromFlash(str_beyond_fn_led_sl);
    printEnter();
    printString(toString(CMD_EXIT_BEYOND_FN_LED));
    printStringFromFlash(str_colon);
    printStringFromFlash(str_exit); // exit
    printEnter();
    printString(toString(CMD_BACK_BEYOND_FN_LED));
    printStringFromFlash(str_colon);
    printStringFromFlash(str_back);
    printEnter();
}

void putKeyindexBeyondFn(uint8_t xCmd, uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, uint8_t xIsDown){
    if(xCmd == CMD_OFF_BEYOND_FN_LED || xCmd == CMD_NL_BEYOND_FN_LED || xCmd == CMD_SL_BEYOND_FN_LED){
        setStep(STEP_INPUT_COMMAND);
        setBeyondFnLed(xCmd-1);
        printBeyondFnLedState();
        printEnter();
    }else if(xCmd == CMD_EXIT_BEYOND_FN_LED){
        setStep(STEP_EXIT);
        stopKeyMapping();
    }else if(xCmd == CMD_BACK_BEYOND_FN_LED){     
        setStep(STEP_BACK);
    }
}
#endif


void initBeyondFn(void){
    /*
     TOGGLE_BEYOND_FN_LED_NL == 1, TOGGLE_BEYOND_FN_LED_SL == 1 : off
     TOGGLE_BEYOND_FN_LED_NL == 0, TOGGLE_BEYOND_FN_LED_SL == 1 : NL
     TOGGLE_BEYOND_FN_LED_NL == 1, TOGGLE_BEYOND_FN_LED_SL == 0 : SL
     */
//    _beyondFnLedEnabled = getToggleOption(EEPROM_ENABLED_OPTION, TOGGLE_BEYOND_FN_LED_NL);
    /*if(
        ((eeprom_read_byte((uint8_t *) EEPROM_ENABLED_OPTION) >> TOGGLE_BEYOND_FN_LED_NL) & 0x01) == OPTION_ON
        && ((eeprom_read_byte((uint8_t *) EEPROM_ENABLED_OPTION) >> TOGGLE_BEYOND_FN_LED_SL) & 0x01) == OPTION_OFF
    ){
        _beyondFnLed = BEYOND_FN_LED_NL;
    }else if(
        ((eeprom_read_byte((uint8_t *) EEPROM_ENABLED_OPTION) >> TOGGLE_BEYOND_FN_LED_NL) & 0x01) == OPTION_OFF
        && ((eeprom_read_byte((uint8_t *) EEPROM_ENABLED_OPTION) >> TOGGLE_BEYOND_FN_LED_SL) & 0x01) == OPTION_ON
    ){
        _beyondFnLed = BEYOND_FN_LED_SL;
    }else{
        _beyondFnLed = BEYOND_FN_LED_OFF;
    }*/

#if 0 //ndef DISABLE_HARDWARE_MENU
    addKeymapperDriver(&driverKeymapperBeyondFn);
#endif
}


static void applyLock(uint8_t *lock){
    if(*lock == LOCK_WILL_SET){
        *lock = LOCK_IS_SET;
        blinkOnce(50);
        _delay_ms(50);
        blinkOnce(100);
    }else if(*lock == LOCK_WILL_NOT_SET){
        *lock = LOCK_NOT_SET;
        blinkOnce(100);
    }
}
// 
void enterFrameForFnControl(void){
    if(isReleaseAll()){
        applyLock(&_isLockKey);
        applyLock(&_isLockWin);
    }
    if(isReleaseAll() && _isReadyQuickMacro){
        _isReadyQuickMacro = false;
        if(_quickMacroIndex < 255) {
            startQuickMacro(_quickMacroIndex);
            _quickMacroIndex = 255;
        }
    }
}
static void __setKeyEnabled(uint8_t *lock){
    if(*lock == LOCK_IS_SET){
        *lock = LOCK_WILL_NOT_SET;
    }else{
        *lock = LOCK_WILL_SET;              
    }
}
void setKeyEnabled(uint8_t xKeyidx, bool xIsDown){
    if(xIsDown) {   // !xIsDown은 오작동한다.
        if(xKeyidx == KEY_LOCK_ALL){
            __setKeyEnabled(&_isLockKey);
        }else if(xKeyidx == KEY_LOCK_WIN){
            __setKeyEnabled(&_isLockWin);
        }
    }
}

bool isKeyEnabled(uint8_t xKeyidx){

    if(_isLockKey == LOCK_IS_SET){
        return false;
    }else if(_isLockWin == LOCK_IS_SET && (xKeyidx == KEY_LGUI || xKeyidx == KEY_RGUI)){
        return false;
    }
    return true;

}

// 키를 누르거나 땔때 FN 및 LED등 을 컨트롤한다.
bool applyFN(uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, bool xIsDown) {

    // dual action key를 기본키 (caps lock 등)으로 반환;
    xKeyidx = getDualActionDefaultKey(xKeyidx);

    if(isFnKey(xKeyidx)) return false;

    if(xIsDown) {

        if((xKeyidx ==  KEY_BEYOND_FN || xKeyidx == KEY_BEYOND_FN3)
        		|| (_isDownExtraFN && xKeyidx == BEYOND_FN_CANCEL_KEY)){ // beyond_fn을 활성화;
            _prevBeyondFnIndex = _beyondFnIndex;
             if( xKeyidx == BEYOND_FN_CANCEL_KEY ) {    // 취소만 가능한 키 
                _beyondFnIndex = LAYER_NORMAL;
             }else{
            	if(_beyondFnIndex == LAYER_NORMAL){
            		if(xKeyidx ==  KEY_BEYOND_FN){
            			_beyondFnIndex = LAYER_FN2;
            		}else{
            			_beyondFnIndex = LAYER_FN3;
            		}
            	}else{
            		if(xKeyidx ==  KEY_BEYOND_FN && !(_beyondFnIndex ==  LAYER_FN2)){
            			_beyondFnIndex = LAYER_FN2;
					}else if(xKeyidx ==  KEY_BEYOND_FN3 && !(_beyondFnIndex ==  LAYER_FN3)){
            			_beyondFnIndex = LAYER_FN3;
					}else{
        				_beyondFnIndex = LAYER_NORMAL;
					}
            	}
             }

#ifdef FN_TOGGLE_TEST
             if(getBeyondFnLed() == BEYOND_FN_LED_OFF){
                 if(_beyondFnIndex == 0){
                    blinkOnce(100);
                 }else{
                    blinkOnce(100);
                    _delay_ms(80);
                    blinkOnce(70);
                 }
             }
             __setBeyondFnLed();
#else
             setLEDIndicate();
#endif
             _prevBeyondFnIndex = _beyondFnIndex;   // 더이상 변화가 없도록;


             if( xKeyidx == BEYOND_FN_CANCEL_KEY ) {    // 키가 작동하도록 1 리턴;
            	 return 1;
			  }
             return 0;
        }else if(_isDownQuickMacro && isEepromMacroKey(xKeyidx)){
            _quickMacroIndex = xKeyidx - KEY_MAC1; 
            _isReadyQuickMacro = true;
            return 0;
        }else if(xKeyidx == EXTRA_FN){
            _isDownExtraFN = true;
        }else if(xKeyidx == KEY_LED_ON_OFF){
        	_ledOff ^= true;
        	if(_ledOff == false){
        		turnOnLedAll();
        	}else{
        		turnOffLedAll();
        	}
        	return 0;
        }else if(xKeyidx == KEY_QUICK_MACRO){
            if(isQuickMacro()){
                stopQuickMacro();
            }else{
                _isDownQuickMacro = true;
            }
            return 0;
        }else{
        	return delegateFnControl(xKeyidx, _isDownExtraFN);
        }

    }else{  // up 

        if(xKeyidx ==  KEY_BEYOND_FN){  // beyond_fn             
             return 0;
        }else if(xKeyidx == EXTRA_FN){
            _isDownExtraFN = false;
        }else if(xKeyidx == KEY_QUICK_MACRO){
            _isDownQuickMacro = false;
            return 0;
        }
    }

    return 1;
}
