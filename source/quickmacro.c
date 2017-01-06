#ifndef KEYMAPPER_C
#define KEYMAPPER_C

#include "timerinclude.h"
#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/eeprom.h>

#include <util/delay.h>
#include <string.h>

// #include "print.h"
#include "quickmacro.h"
#include "keymap.h"
#include "keymatrix.h"
#include "keyindex.h"
#include "ps2avru_util.h"
#include "macrobuffer.h"
#include "main.h"
//#include "ps2main.h"
#include "usbmain.h"
#include "ledrender.h"
#include "keydownbuffer.h"
#include "quickswap.h"
//#include "lazyfn.h"
#include "custommacro.h"
#include "dualaction.h"
#include "bootmapper.h"
#include "fncontrol.h"
#include "esctilde.h"
#include "oddebug.h"

static uint8_t _macroIndex;
static uint8_t _macroBufferIndex;
static uint8_t _macroPressedBuffer[MACRO_SIZE_MAX_HALF];
static uint8_t _macroInputBuffer[MACRO_SIZE_MAX];

static uint8_t _macroDownCount = 0;
static bool _isQuickMacro = false;
static bool _isQuickMacroStopped;
static void __stopQuickMacro(void);

static uint8_t _step;

uint8_t isDeepKeyMapping(void){

	return _isQuickMacro;

}

void enterFrameForMapper(void){

	// for quick macro save
	if(_isQuickMacroStopped && isReleaseAll() && !isActiveMacro())
	{
	    __stopQuickMacro();
	}
}

static void pushCharacter(char *xStr)
{
    // 버퍼에 쌓아두고 모두 출력되기를 기다린다.
    macro_key_t key = charToKey(xStr[0]);
    if(key.mode){
        pushMacroKeyIndex(KEY_LSHIFT);
    }
    pushMacroKeyIndex(key.keyindex);    // 같은 코드를 2개 입력한다. 첫번째는 press, 두번재는 release
    pushMacroKeyIndex(key.keyindex);
    // DEBUG_PRINT(("pushCharacter char %c : %d \n", xStr[0], key.keyindex));
    if(key.mode){
        pushMacroKeyIndex(KEY_LSHIFT);
    }
   
}

void printString(const char *xString)
{
	char c;
	char gChar[1];
	uint8_t i;
	// DEBUG_PRINT(("printString len : %d string : %s \n", strlen(xString), xString));
    for (i = 0; i < strlen(xString); i++) {
        c = xString[i];
        // DEBUG_PRINT(("i = %d, %c ", i, c));
        sprintf(gChar, "%c", c);
    	pushCharacter(gChar);
    	
    }
}

bool isMacroKey(uint8_t xKeyidx){
	if(xKeyidx >= KEY_CST_MAC1 && xKeyidx <= KEY_MAC12){
		return true;
	}
	return false;

}

bool isEepromMacroKey(uint8_t xKeyidx){
	if(xKeyidx >= KEY_MAC1 && xKeyidx <= KEY_MAC12){
		return true;
	}
	return false;

}

// 매크로 적용됐으면 1, 아니면 0 반환;
uint8_t applyMacro(uint8_t xKeyidx){

	uint8_t gMacroIndex;

	if(isMacroKey(xKeyidx)){		
		if(!isActiveMacro()){
		    gMacroIndex = xKeyidx - KEY_CST_MAC1;
		    if(hasCustomMacroAt(gMacroIndex)){
                clearMacroPressedBuffer();
                readCustomMacroAt(gMacroIndex);
		    }

		}else if(isRepeat()){
			clearRepeat();
			stopRepeat();
		}

        return 1;
	}
	return 0;
}


void saveMacro(void){
	if(_macroIndex >= MACRO_NUM) return;

	eeprom_update_block(&_macroInputBuffer, (uint8_t *)(EEPROM_MACRO + (MACRO_SIZE_MAX * _macroIndex)), MACRO_SIZE_MAX);
	_macroIndex = 255;
}

bool isMacroInput(void){
	if(_step == STEP_INPUT_MACRO){
		return true;
	}else{
		return false;
	}
}

static void resetMacroInput(void){	
	memset(_macroInputBuffer, 0, MACRO_SIZE_MAX);
	_macroBufferIndex = 0;
	memset(_macroPressedBuffer, 0, MACRO_SIZE_MAX);
	_macroDownCount = 0;
}

bool isQuickMacro(void){
	return _isQuickMacro;
}

void startQuickMacro(uint8_t xMacroIndex){
	resetMacroInput();
	_macroIndex = xMacroIndex;
	_isQuickMacro = true;
	_isQuickMacroStopped = false;

	blinkOnce(200);
	_delay_ms(100);
	blinkOnce(100);
}

static void __stopQuickMacro(void){

//  DBG1(0xef, (uchar *)&_macroBufferIndex, 1);
    _isQuickMacroStopped = false;
    saveMacro();
    _macroIndex = 255;

    blinkOnce(100);
}

void stopQuickMacro(void){
    _isQuickMacro = false;
    _isQuickMacroStopped = true;
}

static void stopMacroInput(void){
//	DBG1(0xee, (uchar *)&_macroBufferIndex, 1);
	if(_isQuickMacro){
	    stopQuickMacro();
	}

}

static uint8_t putMacro(uint8_t xKeyidx, uint8_t xIsDown){
    int gIdx;

	if(xKeyidx >= KEY_MAX) return KEY_NONE;		// 매크로 입력시 키값으로 변환할 수 없는 특수 키들은 중단;

	if(xIsDown){
		if(_macroDownCount >= MACRO_SIZE_MAX_HALF){	// 매크로 크기의 절반이 넘은 키 다운은 제외 시킨다. 그래야 나머지 공간에 up 데이터를 넣을 수 있으므로.
			return KEY_NONE;
		}
		++_macroDownCount;
	    DBG1(0x07, (uchar *)&xKeyidx, 1);  
	   
	    append(_macroPressedBuffer, xKeyidx);
	    
	}else{
	    gIdx = findIndex(_macroPressedBuffer, xKeyidx);
	    // 릴리즈시에는 프레스 버퍼에 있는 녀석만 처리; 버퍼에 없는 녀석은 16키 이후의 키이므로 제외;

	    if(gIdx == -1){
	    	return KEY_NONE;
	    }
	    delete(_macroPressedBuffer, gIdx);
	    DBG1(0x08, (uchar *)&xKeyidx, 1);  
	}

	_macroInputBuffer[_macroBufferIndex] = xKeyidx;
	++_macroBufferIndex;

	DBG1(0x09, (uchar *)&_macroInputBuffer, strlen((char *)_macroInputBuffer));  

	// MACRO_SIZE_MAX개를 채웠다면 종료;
	if(_macroBufferIndex >= MACRO_SIZE_MAX){
		// _macroIndex 위치에 저장;
	
		stopMacroInput();

		return xKeyidx;
	}


	return xKeyidx;
}

uint8_t putKeyindex(uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, uint8_t xIsDown)
{
    xKeyidx = getDualActionDownKeyIndexWhenIsCompounded(xKeyidx, false);

    return	putMacro(xKeyidx, xIsDown);
}

#endif
