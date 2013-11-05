#ifndef MACROBUFFER_H
#define MACROBUFFER_H

#include <stdlib.h>
#include <string.h>
#include <avr/eeprom.h>
#include "ps2avru_util.h"


#define MOD_SHIFT_LEFT  (1 << 1)
/**
 * This structure can be used as a container for a single 'key'. It consists of
 * the key-code and the modifier-code.
 */
typedef struct {
    uint8_t mode;
    uint8_t keycode;
} Key;


#define MESSAGE_SIZE_MAX 256
#define MACRO_SIZE_MAX 48
#define MACRO_SIZE_MAX_HALF 24

extern uint8_t _pressedBuffer[MACRO_SIZE_MAX];

// Queue operation -> push, pop
extern void pushM(uint8_t item) ;

extern uint8_t popM(void);

// 매크로 버퍼에서 키값을 가져와 프레스/업을 확인하여 Key 를 반환한다.
extern Key popMWithKey(void);

/*const uint8_t * getPressedBuffer(void){
    return _pressedBuffer;
}*/

extern uint8_t isEmptyM(void);

// 새로운 매크로 시작전에 초기화;
extern void clearMacroPressedBuffer(void);

// 현재 매크로가 진행중인지 확인, 각 인터페이스에서 setMacroProcessEnd()로 표시해준다.
extern uint8_t isMacroProcessEnd(void);
extern void setMacroProcessEnd(uint8_t xIsEnd);


extern const char * toString(uint8_t xInt);


/**
 * Convert an ASCII-character to the corresponding key-code and modifier-code
 * combination.
 * \parm character ASCII-character to convert
 * \return structure containing the combination
 */

extern Key charToKey(char character);

#endif