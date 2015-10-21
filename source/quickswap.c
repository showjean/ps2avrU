#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>
#include <util/delay.h>

// #include "print.h"
#include "quickswap.h"
#include "keymap.h"
#include "hardwareinfo.h"
#include "keydownbuffer.h"
#include "ledrender.h"
#include "main.h"

static uint8_t quickSwapMode;
static uint8_t enabledQuickSwapCount;
static int quickSwapCountMax;
static int _quickSwapCount;
static void applyQuickSwap(void);

void initQuickSwap(void){
		
	quickSwapMode = eeprom_read_byte((uint8_t *)EEPROM_QUICK_SWAP); 
	if(quickSwapMode == 0xFF) quickSwapMode = 0;
		
	quickSwapCountMax = setDelay(QUICKSWAP_COUNT_MAX);
}
/**
	전달되는 키인덱스를 현재 퀵스왑 형식에 맞게 변환;
*/
uint8_t getQuickSwapKeyindex(uint8_t xKeyidx){

	//apply quick swap
	if((quickSwapMode & QUICKSWAP_ALT_GUI) == QUICKSWAP_ALT_GUI){
		if(xKeyidx == KEY_LALT){
			xKeyidx = KEY_LGUI;
		}else if(xKeyidx == KEY_LGUI){
			xKeyidx = KEY_LALT;
		}else if(xKeyidx == KEY_RALT){
			xKeyidx = KEY_RGUI;			
		}else if(xKeyidx == KEY_RGUI){
			xKeyidx = KEY_RALT;			
		}
	}
	return xKeyidx;
}
static void applyQuickSwap(void) {
	static uint8_t prevModifier = 0;

	if(getDownBufferAt(0) > 0){
		enabledQuickSwapCount = 0;
		return;
	}
	
	uint8_t gModi = getModifierDownBuffer();

	if(gModi != prevModifier) {

        if(gModi == 0x2C || gModi == 0xE0 || gModi == 0x68 || gModi == 0xA4){	// ALT + GUI + R SHIFT
            _quickSwapCount = 0;
            enabledQuickSwapCount = COUNT_TYPE_QUICKSWAP_ALT_GUI;
        }else{
            enabledQuickSwapCount = 0;
        }

        prevModifier = gModi;

    }

//}

//static void countQuickSwapEnabled(void){
	if(enabledQuickSwapCount && ++_quickSwapCount > quickSwapCountMax){
		if(enabledQuickSwapCount == COUNT_TYPE_QUICKSWAP_ALT_GUI){
			blinkOnce(50);
			if((quickSwapMode & QUICKSWAP_ALT_GUI) == QUICKSWAP_ALT_GUI){			
				quickSwapMode &= ~QUICKSWAP_ALT_GUI;
			}else{
				quickSwapMode |= QUICKSWAP_ALT_GUI;		
				_delay_ms(50);
				blinkOnce(100);
			}
		}		
		eeprom_update_byte((uint8_t *)EEPROM_QUICK_SWAP, quickSwapMode);
		enabledQuickSwapCount = 0;
	}

}

void enterFrameForQuickSwap(void){
	applyQuickSwap();

//	countQuickSwapEnabled();
}
