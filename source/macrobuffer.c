#ifndef MACROBUFFER_C
#define MACROBUFFER_C

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <avr/eeprom.h>

#include "keymap.h" 
#include "macrobuffer.h"
#include "ps2avru_util.h"
#include "hardwareinfo.h"
#include "custommacro.h"
#include "oddebug.h"

static uint8_t QUEUE[MESSAGE_SIZE_MAX];
static int rear = 0, front = 0;

static uint8_t _pressedBuffer[MACRO_SIZE_MAX] = {0};

void clearMacroKeyIndex(void){
	rear = 0;
	front = 0;
	memset(QUEUE, 0, MESSAGE_SIZE_MAX);
	memset(_pressedBuffer, 0, MACRO_SIZE_MAX);
}
// Queue operation -> push, pop
// keyindex를 저장한다.
void pushMacroKeyIndex(uint8_t item) {
    
    rear = (rear+1) % MESSAGE_SIZE_MAX;
    if(front == rear) {
        rear = (rear!=0) ? (rear-1):(MESSAGE_SIZE_MAX-1);
        return;
    }
    QUEUE[rear] = item;
}

uint8_t popMacroKeyIndex(void) {
    if(front == rear) {
        return 0;
    }
    front = (front+1) % MESSAGE_SIZE_MAX;

    return QUEUE[front];
}
static bool _isRepeat = false;
bool isRepeat(void){
	return _isRepeat;
}
void stopRepeat(void){
	_isRepeat = false;
}
void clearRepeat(void){
	clearMacroKeyIndex();
	closeCustomMacro();
}

// 매크로 버퍼에서 키값을 가져와 프레스/업을 확인하여 Key 를 반환한다.
macro_key_t popMacroKey(void) {
    macro_key_t gKey;
    int gIdx;

    gKey.mode = MACRO_KEY_UP; // down = 1, up = 0;
    gKey.keyindex = popMacroKeyIndex();

//    DBG1(0x76, (void *)&gKey.keyindex, 1);

	/*
	 * 매크로키가 발견되면 현재 매크로 중단,
	 * 새 매크로 시작;
	 */
//	if(gKey.keyindex >= KEY_CST_MAC1 && gKey.keyindex <= KEY_CST_MAC12){
    if(isMacroKey(gKey.keyindex)){
		DBG1(0x77, (void *)&gKey.keyindex, 1);

		clearRepeat();

		readCustomMacroAt(gKey.keyindex - KEY_CST_MAC1);

		_isRepeat = true;

		gKey.keyindex = 0;
		return gKey;
	}

    if(gKey.keyindex == 0) {
        memset(_pressedBuffer, 0, MACRO_SIZE_MAX);
        return gKey;
    }

    //
    gIdx = findIndex(_pressedBuffer, gKey.keyindex);
    if(gIdx >= 0){
        // already pressed
        delete(_pressedBuffer, gIdx);
    }else{
        append(_pressedBuffer, gKey.keyindex);
        gKey.mode = MACRO_KEY_DOWN;
    }


    return gKey;
}

bool isEmptyMacroKeyIndex(void) {
    if(front == rear)
        return true;
    else
        return false;
}

// 새로운 매크로 시작전에 초기화;
void clearMacroPressedBuffer(void){
    if(isEmptyMacroKeyIndex()) memset(_pressedBuffer, 0, MACRO_SIZE_MAX);
}

// 현재 매크로가 진행중인지 확인, 
// 매크로 시작시 true, 각 인터페이스에서 종료시 false로 표시해준다.
bool isActiveMacro(void)
{
    return !isEmptyMacroKeyIndex() || isActiveCustomMacro();
}

const char * toString(uint8_t xInt)
{        
    static char gStr[3]; 
    itoa(xInt, gStr, 10);
    return gStr;
}


/**
 * Convert an ASCII-character to the corresponding key-code and modifier-code
 * combination.
 * \parm character ASCII-character to convert
 * \return structure containing the combination
 */

macro_key_t charToKey(char character) {
    macro_key_t key;
    // initialize with reserved values
    key.mode = KEY_NONE;
    key.keyindex = KEY_NONE;
    if ((character >= 'a') && (character <= 'z')) {
        // a..z
        key.keyindex = (character - 'a') + 0x04;
        // DEBUG_PRINT(("a...z"));
    } else if ((character >= 'A') && (character <= 'Z')) {
        // A..Z
        key.mode = MOD_SHIFT_LEFT;
        key.keyindex = (character - 'A') + 0x04;
        // DEBUG_PRINT(("A...Z"));
    } else if ((character >= '1') && (character <= '9')) {
        // 1..9
        key.keyindex = (character - '1') + 0x1E;
        // DEBUG_PRINT(("1...9"));
    }
    // we can't map the other characters directly, so we switch...
    switch (character) {
        case '0':
            key.keyindex = KEY_0; break;
        case '!':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_1; break;
        case '@':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_2; break;
        case '#':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_3; break;
        case '$':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_4; break;
        case '%':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_5; break;
        case '^':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_6; break;
        case '&':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_7; break;
        case '*':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_8; break;
        case '(':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_9; break;
        case ')':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_0; break;
        case ' ':
            key.keyindex = KEY_SPACE; break;
        case '-':
            key.keyindex = KEY_MINUS; break;
        case '_':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_MINUS; break;
        case '=':
            key.keyindex = KEY_EQUAL; break;
        case '+':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_EQUAL; break;
        case '[':
            key.keyindex = KEY_LBR; break;
        case '{':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_LBR; break;
        case ']':
            key.keyindex = KEY_RBR; break;
        case '}':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_RBR; break;
        case '\\':
            key.keyindex = KEY_BKSLASH; break;
        case '|':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_BKSLASH; break;
        case ';':
            key.keyindex = KEY_COLON; break;
        case ':':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_COLON; break;
        case '\'':
            key.keyindex = KEY_QUOTE; break;
        case '"':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_QUOTE; break;
        case '`':
            key.keyindex = KEY_HASH; break;
        case '~':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_HASH; break;
        case ',':
            key.keyindex = KEY_COMMA; break;
        case '<':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_COMMA; break;
        case '.':
            key.keyindex = KEY_DOT; break;
        case '>':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_DOT; break;
        case '/':
            key.keyindex = KEY_SLASH; break;
        case '?':
            key.mode = MOD_SHIFT_LEFT;
            key.keyindex = KEY_SLASH; break;
    }
    if (key.keyindex == KEY_NONE) {
        // still reserved? WTF? return question mark...
        key.mode = MOD_SHIFT_LEFT;
        key.keyindex = KEY_SLASH;
    }
    return key;
} 

#endif
