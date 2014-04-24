#ifndef KEYDOWNBUFFER_C
#define KEYDOWNBUFFER_C

#include <stdio.h>
#include <string.h>
#include "print.h"
#include "keymap.h"
#include "keymatrix.h"
#include "keydownbuffer.h"
#include "ps2avru_util.h"
#include "oddebug.h"

static uint8_t _downBuffer[DOWN_BUFFER_SIZE];
static uint8_t _downModifyBuffer;
static uint8_t _downBufferIndex;

/**
	현재 눌려진 키들을 저장하고 있다. 최대 6+1개;

	각 기능들은 이 버퍼를 이용해서 키의 상태를 파악한다.
*/
void pushDownBuffer(uint8_t xKeyidx, bool xIsDown){

//    uint8_t gLen;
	int gIdx;
	if(xIsDown){
		if (xKeyidx > KEY_Modifiers && xKeyidx < KEY_Modifiers_end) { // Is this a modifier key?
			_downModifyBuffer |= getModifierBit(xKeyidx); // modmask[xKeyidx - (KEY_Modifiers + 1)];
//			DBG1(0x34, (uchar *)&_downBuffer, DOWN_BUFFER_SIZE);
		}else{ // keycode should be added to report
//	            gLen = strlen((char *)_downBuffer);
			if (_downBufferIndex >= DOWN_BUFFER_SIZE) { // too many keycodes
//				DBG1(0x39, (uchar *)&_downBuffer, DOWN_BUFFER_SIZE);
			} else {
//	                append(reportBuffer, xKeyidx);
				gIdx = findIndex(_downBuffer, xKeyidx);
//				DBG1(0x34, (uchar *)&gIdx, 2);
				if(gIdx == -1){
					_downBuffer[_downBufferIndex] = xKeyidx;
					++_downBufferIndex;
				}

//				DBG1(0x24, (uchar *)&_downBuffer, DOWN_BUFFER_SIZE);
			}

		}
	}else{

		if (xKeyidx > KEY_Modifiers && xKeyidx < KEY_Modifiers_end) { // Is this a modifier key?
			_downModifyBuffer &= ~(getModifierBit(xKeyidx));
//			DBG1(0x35, (uchar *)&_downBuffer, DOWN_BUFFER_SIZE);
		}else{ // keycode should be added to report
			gIdx = findIndex(_downBuffer, xKeyidx);
//			DBG1(0x35, (uchar *)&gIdx, 1);
			if(gIdx >= 0){
				delete(_downBuffer, gIdx);
				--_downBufferIndex;
			}
//			DBG1(0x25, (uchar *)&_downBuffer, DOWN_BUFFER_SIZE);


		}
	}

}

void initKeyDownBuffer(void){
	_downBufferIndex = 0;
	_downModifyBuffer = 0;
	memset(_downBuffer, 0, DOWN_BUFFER_SIZE);
}

uint8_t getDownBufferAt(uint8_t xIdx){
	if(xIdx >= DOWN_BUFFER_SIZE) return 0;
	return _downBuffer[xIdx];
}

uint8_t getModifierDownBuffer(void){
	return _downModifyBuffer;
}

#endif
