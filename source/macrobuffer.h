#ifndef MACROBUFFER_H
#define MACROBUFFER_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <avr/eeprom.h>
#include "ps2avru_util.h"


#define MOD_SHIFT_LEFT  (1 << 1)
#define MACRO_KEY_DOWN  	1
#define MACRO_KEY_UP	  	2
/**
 * This structure can be used as a container for a single 'key'. It consists of
 * the key-code and the modifier-code.
 */
typedef struct {
    uint8_t mode;
    uint8_t keyindex;
} macro_key_t;


#define MESSAGE_SIZE_MAX 300

// Queue operation -> push, pop
void pushM(uint8_t item);
uint8_t popM(void);
bool isEmptyM(void);

// 매크로 버퍼에서 키값을 가져와 프레스/업을 확인하여 Key 를 반환한다.
macro_key_t popMWithKey(void);

// 새로운 매크로 시작전에 초기화;
void clearMacroPressedBuffer(void);

// 현재 매크로가 진행중인지 확인, 
// 매크로 시작시 true, 각 인터페이스에서 종료시 false로 표시해준다.
bool isActiveMacro(void);

const char * toString(uint8_t xInt);

/**
 * Convert an ASCII-character to the corresponding key-code and modifier-code
 * combination.
 * \parm character ASCII-character to convert
 * \return structure containing the combination
 */

macro_key_t charToKey(char character);

#endif
