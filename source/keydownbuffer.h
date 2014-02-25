#ifndef KEYDOWNBUFFER_H
#define KEYDOWNBUFFER_H

#include <stdio.h>

#define DOWN_BUFFER_SIZE            7
#define DOWN_BUFFER_MODIFIER_INDEX  0
#define DOWN_BUFFER_START_INDEX     1
/**
	현재 눌려진 키 index들을 저장하고 있다. 최대 6+1개;
	이 인덱스는 keymap.h에 정의한 기본 값이다.
*/
extern void pushDownBuffer(uint8_t xKeyidx);
extern void clearDownBuffer(void);
extern uint8_t * getDownBuffer(void);
extern uint8_t getDownBufferAt(uint8_t xIdx);
extern uint8_t getModifierDownBuffer(void);

#endif