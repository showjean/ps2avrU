#ifndef MACROBUFFER_C
#define MACROBUFFER_C

#include <stdlib.h>
#include <string.h>
#include <avr/eeprom.h>

#include "keymap.h" 
#include "macrobuffer.h"
#include "ps2avru_util.h"

static uint8_t _isMacroProcessEnd = 0;
static uint8_t QUEUE_M[MESSAGE_SIZE_MAX];
static int rearm = 0, frontm = 0;

uint8_t _pressedBuffer[MACRO_SIZE_MAX] = {0};

// Queue operation -> push, pop
void pushM(uint8_t item) {
    
    rearm = (rearm+1) % MESSAGE_SIZE_MAX;
    if(frontm == rearm) {
        rearm = (rearm!=0) ? (rearm-1):(MESSAGE_SIZE_MAX-1);
        return;
    }
    QUEUE_M[rearm] = item;
}

uint8_t popM(void) {
    if(frontm == rearm) {
        return 0;
    }
    frontm = (frontm+1) % MESSAGE_SIZE_MAX;

    return QUEUE_M[frontm];
}

// 매크로 버퍼에서 키값을 가져와 프레스/업을 확인하여 Key 를 반환한다.
Key popMWithKey(void) {
    Key gKey;
    int gIdx;
    int gLen;

    gKey.mode = 0; // down = 1, up = 0;
    gKey.keycode = popM();

    if(gKey.keycode == 0) {
        memset(_pressedBuffer, 0, MACRO_SIZE_MAX);
        return gKey;
    }

    //
    gLen = strlen((char *)_pressedBuffer);
    gIdx = findIndex(_pressedBuffer, gLen, gKey.keycode);
    // DEBUG_PRINT(("findIndex gIdx :: %d , len : %d \n", gIdx, gLen));
    if(gIdx > -1){
        // is pressed
        delete(_pressedBuffer, gIdx);
    }else{
        append(_pressedBuffer, gKey.keycode);
        gKey.mode = 1;
    }

    return gKey;
}

/*const uint8_t * getPressedBuffer(void){
    return _pressedBuffer;
}*/

uint8_t isEmptyM(void) {
    if(frontm == rearm)
        return 1;
    else
        return 0;
}

// 프레스 버퍼가 있는 동안 릴리즈 하지 않으면 (usb에서) 리피트가 되버린다.
/*uint8_t isEmptyMWithPressedBuffer(void){
    if(isEmptyM() && strlen(_pressedBuffer) == 0){
        return 1;
    }else{
        return 0;
    }
}*/

// 새로운 매크로 시작전에 초기화;
void clearMacroPressedBuffer(void){
    if(isEmptyM()) memset(_pressedBuffer, 0, MACRO_SIZE_MAX);
}


// 현재 매크로가 진행중인지 확인, 각 인터페이스에서 setMacroProcessEnd()로 표시해준다.
uint8_t isMacroProcessEnd(void)
{
    return _isMacroProcessEnd && isEmptyM();
}
void setMacroProcessEnd(uint8_t xIsEnd)
{
    _isMacroProcessEnd = xIsEnd;
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

Key charToKey(char character) {
    Key key;
    // initialize with reserved values
    key.mode = KEY_NONE;
    key.keycode = KEY_NONE;
    if ((character >= 'a') && (character <= 'z')) {
        // a..z
        key.keycode = (character - 'a') + 0x04;
        // DEBUG_PRINT(("a...z"));
    } else if ((character >= 'A') && (character <= 'Z')) {
        // A..Z
        key.mode = MOD_SHIFT_LEFT;
        key.keycode = (character - 'A') + 0x04;
        // DEBUG_PRINT(("A...Z"));
    } else if ((character >= '1') && (character <= '9')) {
        // 1..9
        key.keycode = (character - '1') + 0x1E;
        // DEBUG_PRINT(("1...9"));
    }
    // we can't map the other characters directly, so we switch...
    switch (character) {
        case '0':
            key.keycode = KEY_0; break;
        case '!':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_1; break;
        case '@':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_2; break;
        case '#':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_3; break;
        case '$':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_4; break;
        case '%':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_5; break;
        case '^':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_6; break;
        case '&':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_7; break;
        case '*':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_8; break;
        case '(':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_9; break;
        case ')':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_0; break;
        case ' ':
            key.keycode = KEY_SPACE; break;
        case '-':
            key.keycode = KEY_MINUS; break;
        case '_':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_MINUS; break;
        case '=':
            key.keycode = KEY_EQUAL; break;
        case '+':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_EQUAL; break;
        case '[':
            key.keycode = KEY_LBR; break;
        case '{':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_LBR; break;
        case ']':
            key.keycode = KEY_RBR; break;
        case '}':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_RBR; break;
        case '\\':
            key.keycode = KEY_BKSLASH; break;
        case '|':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_BKSLASH; break;
        /*case '#':
            key.keycode = KEY_hash; break;
        case '@':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_hash; break;*/
        case ';':
            key.keycode = KEY_COLON; break;
        case ':':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_COLON; break;
        case '\'':
            key.keycode = KEY_QUOTE; break;
        case '"':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_QUOTE; break;
        case '`':
            key.keycode = KEY_HASH; break;
        case '~':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_HASH; break;
        case ',':
            key.keycode = KEY_COMMA; break;
        case '<':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_COMMA; break;
        case '.':
            key.keycode = KEY_DOT; break;
        case '>':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_DOT; break;
        case '/':
            key.keycode = KEY_SLASH; break;
        case '?':
            key.mode = MOD_SHIFT_LEFT;
            key.keycode = KEY_SLASH; break;
    }
    if (key.keycode == KEY_NONE) {
        // still reserved? WTF? return question mark...
        key.mode = MOD_SHIFT_LEFT;
        key.keycode = KEY_SLASH;
    }
    return key;
} 

#endif