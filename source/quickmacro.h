#ifndef QUICKMACRO_H
#define QUICKMACRO_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "main.h"
#include "hardwareinfo.h"
#include "keymap.h"
#include "keymatrix.h"
#include "ps2avru_util.h"
#include "macrobuffer.h"
#include "bootmapper.h"

#define KEY_MAPPING_COUNT_MAX 2500

#define STEP_INPUT_MACRO 11


// event loop
void enterFrameForMapper(void);

uint8_t isDeepKeyMapping(void);

void printString(const char *xString);

// 매크로 등록중인지 확인;
bool isMacroInput(void);
bool isMacroKey(uint8_t xKeyidx);
bool isEepromMacroKey(uint8_t xKeyidx);
// 매크로 적용됐으면 1, 아니면 0 반환;
uint8_t applyMacro(uint8_t xKeyidx);
uint8_t putKeyindex(uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, uint8_t xIsDown);
//void readMacro(uint8_t xMacroIndex);

bool isQuickMacro(void);
void startQuickMacro(uint8_t xMacroIndex);
void stopQuickMacro(void);

#endif
