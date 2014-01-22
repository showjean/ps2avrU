#ifndef MACROBUFFER_H
#define MACROBUFFER_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <avr/eeprom.h>
#include "ps2avru_util.h"


#define MOD_SHIFT_LEFT  (1 << 1)
#define MACRO_KEY_DOWN  	1
#define MACRO_KEY_UP	  	0
/**
 * This structure can be used as a container for a single 'key'. It consists of
 * the key-code and the modifier-code.
 */
typedef struct {
    uint8_t mode;
    uint8_t keyindex;
} macro_key_t;


#define MACRO_NUM 12
#define MESSAGE_SIZE_MAX 256
#define MACRO_SIZE_MAX 48
#define MACRO_SIZE_MAX_HALF 24

// Queue operation -> push, pop
extern void pushM(uint8_t item) ;

extern uint8_t popM(void);

// 매크로 버퍼에서 키값을 가져와 프레스/업을 확인하여 Key 를 반환한다.
extern macro_key_t popMWithKey(void);

extern bool isEmptyM(void);

// 새로운 매크로 시작전에 초기화;
extern void clearMacroPressedBuffer(void);

// 현재 매크로가 진행중인지 확인, 각 인터페이스에서 setMacroProcessEnd()로 표시해준다.
extern bool isMacroProcessEnd(void);
extern void setMacroProcessEnd(bool xIsEnd);


extern const char * toString(uint8_t xInt);


/**
 * Convert an ASCII-character to the corresponding key-code and modifier-code
 * combination.
 * \parm character ASCII-character to convert
 * \return structure containing the combination
 */

extern macro_key_t charToKey(char character);

#endif