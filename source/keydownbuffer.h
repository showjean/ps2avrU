#ifndef KEYDOWNBUFFER_H
#define KEYDOWNBUFFER_H

#include <stdio.h>

#define DOWN_BUFFER_SIZE            6
/**
	현재 눌려진 키 index들을 저장하고 있다. 최대 6+1개;
	이 인덱스는 keymap.h에 정의한 기본 값이다.
*/
void pushDownBuffer(uint8_t xKeyidx, bool xIsDown);
void initKeyDownBuffer(void);
uint8_t getDownBufferAt(uint8_t xIdx);
uint8_t getModifierDownBuffer(void);

#endif
