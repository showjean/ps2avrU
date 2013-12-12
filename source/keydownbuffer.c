#ifndef KEYDOWNBUFFER_C
#define KEYDOWNBUFFER_C

#include <stdio.h>
#include "print.h"
#include "keymap.h"
#include "keymatrix.h"

#define DOWN_BUFFER_SIZE 			7
#define DOWN_BUFFER_MODIFIER_INDEX 	0

static uint8_t _downBuffer[DOWN_BUFFER_SIZE];
static uint8_t _downBufferIndex;

/**
	현재 눌려진 키들을 저장하고 있다. 최대 6+1개;

	각 기능들은 이 버퍼를 이용해서 키의 상태를 파악한다.
*/
void pushDownBuffer(uint8_t xKeyidx){
	if (xKeyidx > KEY_Modifiers && xKeyidx < KEY_Modifiers_end) { 
        _downBuffer[DOWN_BUFFER_MODIFIER_INDEX] |= modmask[xKeyidx - (KEY_Modifiers + 1)];
        return;
    }

	if(_downBufferIndex >= DOWN_BUFFER_SIZE) return;

	_downBuffer[_downBufferIndex] = xKeyidx;
	// DEBUG_PRINT(("_downBufferIndex= %d, _downBuffer[_downBufferIndex]= %d \n", _downBufferIndex, _downBuffer[_downBufferIndex]));
	++_downBufferIndex;
}

void clearDownBuffer(void){
	_downBufferIndex = 1;
	memset(_downBuffer, 0, DOWN_BUFFER_SIZE);
}

uint8_t * getDownBuffer(void){
	return _downBuffer;
}

uint8_t getDownBufferAt(uint8_t xIdx){
	if(xIdx >= DOWN_BUFFER_SIZE) return 0;
	return _downBuffer[xIdx];
}

uint8_t getModifierDownBuffer(void){
	return _downBuffer[DOWN_BUFFER_MODIFIER_INDEX];
}

#endif