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
static void applyQuickSwap() {
	static uint8_t prevModifier = 0;
	static uint8_t prevKey = 0;

	// 작동키 이외의 키들이 눌려 있을 경우는 취소;
	if(getDownBufferAt(1) > 0){
		enabledQuickSwapCount = 0;
		return;
	}
	
	// ALT <-> GUI
	uint8_t gModi = getModifierDownBuffer();
	uint8_t gKey = getDownBufferAt(0);

	if(gModi == 0) prevModifier = 0;
	if(gKey == 0) prevKey = 0;

	if(gModi != prevModifier && prevKey != gKey) //return;	// 같은 모디 키를 누르고 있을 때는 카운트가 초기화 되지 않도록 스킵;
	{
		if((gModi == 0x0C || gModi == 0xC0 || gModi == 0x48 || gModi == 0x84) && gKey == KEY_ENTER){	// ALT + GUI
			_quickSwapCount = 0;
			enabledQuickSwapCount = COUNT_TYPE_QUICKSWAP_ALT_GUI;
		}else{
			enabledQuickSwapCount = 0;
		}

		prevModifier = gModi;
		prevKey = gKey;
	}

	if (enabledQuickSwapCount && ++_quickSwapCount > quickSwapCountMax) {
		if (enabledQuickSwapCount == COUNT_TYPE_QUICKSWAP_ALT_GUI) {
			blinkOnce(50);
			if ((quickSwapMode & QUICKSWAP_ALT_GUI) == QUICKSWAP_ALT_GUI) {
				quickSwapMode &= ~QUICKSWAP_ALT_GUI;
			} else {
				quickSwapMode |= QUICKSWAP_ALT_GUI;
				_delay_ms(50);
				blinkOnce(100);
			}
		}
		eeprom_write_byte((uint8_t *) EEPROM_QUICK_SWAP, quickSwapMode);
		enabledQuickSwapCount = 0;
	}
}

/*static void countQuickSwapEnabled(void){
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
		eeprom_write_byte((uint8_t *)EEPROM_QUICK_SWAP, quickSwapMode);
		enabledQuickSwapCount = 0;
	}

}*/

void enterFrameForQuickSwap(void){
	applyQuickSwap();

//	countQuickSwapEnabled();
}
