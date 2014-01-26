#ifndef QUICKSWAP_C
#define QUICKSWAP_C

#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#include "print.h"
#include "quickswap.h"
#include "keymap.h"
#include "hardwareinfo.h"
#include "keydownbuffer.h"
#include "ledrender.h"
#include "keymain.h"

static uint8_t quickSwapMode;
static uint8_t enabledQuickSwapCount;
static int quickSwapCountMax;
static int _quickSwapCount;

void initQuickSwap(void){
		
	quickSwapMode = eeprom_read_byte((uint8_t *)EEPROM_QUICK_SWAP); 
	if(quickSwapMode == 0xFF) quickSwapMode = 0;

	/*if(INTERFACE == INTERFACE_PS2 || INTERFACE == INTERFACE_PS2_USER){		
		quickSwapCountMax = QUICKSWAP_COUNT_MAX >> 1;	// ps2의 경우 USB보다 대기 시간이 길어서 반으로 줄여줌;
	}else{
		quickSwapCountMax = QUICKSWAP_COUNT_MAX;
	}*/
		
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
static void applyQuickSwap(uint8_t xModi) {
	static uint8_t prevModifier = 0;

	if(xModi == prevModifier) return;

	// DEBUG_PRINT(("xModi= %d, quickSwapMode= %d \n", xModi, quickSwapMode));

	// 약 5초간 입력이 지속되면 키매핑 모드로
	if(xModi == 0x0C || xModi == 0xC0 || xModi == 0x48 || xModi == 0x84){	// ALT + GUI
		_quickSwapCount = 0;
		enabledQuickSwapCount = COUNT_TYPE_QUICKSWAP_ALT_GUI;
	}else{		
		enabledQuickSwapCount = 0;
	}

	prevModifier = xModi;

}

static void countQuickSwapEnabled(void){	
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

}

void enterFrameForQuickSwap(void){
	applyQuickSwap(getModifierDownBuffer());

	countQuickSwapEnabled();
}

#endif