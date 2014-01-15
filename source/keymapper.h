#ifndef KEYMAPPER_H
#define KEYMAPPER_H

#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "keymain.h"
#include "hardwareinfo.h"
#include "keymap.h"
#include "keymatrix.h"
#include "ps2avru_util.h"
#include "macrobuffer.h"
#include "bootMapper.h"

#ifdef DEBUG_KEYMAPPER_H
	#define KEY_MAPPING_COUNT_MAX 500
#else
	#define KEY_MAPPING_COUNT_MAX 5000
#endif

// mapping
#define STEP_NOTHING 0
#define STEP_CHOOSE_LAYER 1
#define STEP_CHOOSE_KEY 2
#define STEP_INPUT_KEYCODE 3
#define STEP_SAVE_END_MAPPING 4
#define STEP_CANCEL_MAPPING 5
#define STEP_RESET_MAPPING 6
#define STEP_INPUT_COMMAND 7
#define STEP_SELECT_MODE 8
// macro
#define STEP_SELECT_INDEX 9
#define STEP_EXIT_MACRO 10
#define STEP_INPUT_MACRO 11
#define STEP_CLEAR_ALL_MACRO 12
#define STEP_CLEAR_SELECT_INDEX 13
// exit
#define STEP_EXIT 100
#define STEP_BACK 110
#define STEP_BOOT_MAPPER 120

// wait
#define WAIT_NOTHING 0
#define WAIT_WELCOME 10
#define WAIT_SAVE 11
#define WAIT_CLEAR_ALL_MACRO 12
#define WAIT_CLEAR_MACRO 13

// mode select
#define SEL_MAPPING 1
#define SEL_MACRO 2
#define SEL_TOGGLE_LAZY_FN 3
#define SEL_EXIT 4
#define SEL_BOOT_MAPPER 9

// 입력되는 키와 직접 매칭; (숫자 1키를 누르면 CMD_CHOOSE_LAYER...)
// mapping
#define CMD_CHOOSE_LAYER 1
#define CMD_SELECT_KEY 2
#define CMD_SAVE_AND_EXIT 3
#define CMD_CANCEL_WITHOUT_SAVE 4
#define CMD_RESET_TO_DEFAULT 9
// macro
#define CMD_SELECT_INDEX 1
#define CMD_CLEAR_MACRO 2
#define CMD_EXIT_MACRO 3
#define CMD_CLEAR_ALL_MACRO 9
// lazy fn
#define CMD_TOGGLE_LAZY_FN 1
// back
#define CMD_BACK 6

#define COUNT_TYPE_NONE		 	0
#define COUNT_TYPE_COUNTING 	1
#define COUNT_TYPE_KEYMAPPER 	2

#define KEYMAPPER_START_KEYS	0x27	// KEY_LCTRL + KEY_LSHIFT + KEY_LALT + KEY_RSHIFT	키를 수정하면 ps2의 prepareKeyMappingPs2를 함께 수정해줘야 한다.

extern void initKeymapper(void);

extern uint8_t isKeyMapping(void);

// 부팅시 키 매핑 시작
// extern uint8_t isReadyKeyMappingOnBoot(void);
extern void readyKeyMappingOnBoot(void);
extern void startKeyMappingOnBoot(void);

extern void printString(const char *xString);

// event loop
extern void enterFrameForMapper(void);

// 키들을 순서대로 나열한 인덱스를 반환. <키코드가 아님!>
extern uint8_t getCurrentKeyindex(uint8_t xLayer, uint8_t xRow, uint8_t xCol);
// 매크로 등록중인지 확인;
extern uint8_t isMacroInput(void);
// 매크로 적용됐으면 1, 아니면 0 반환;
extern uint8_t applyMacro(uint8_t xKeyidx);

extern void putKeyCode(uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, uint8_t xIsDown);


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
// extern void setWillStartKeyMapping(void);
// extern uint8_t isDeepKeyMapping(void);
// extern void applyKeyMapping(uint8_t xModi);
/**
 진입키(Left - ctrl+alt+shift)가 입력되면 매핑 시작을 준비한다.
*/
// extern void prepareKeyMapping(void);
/**
매핑 준비가 되었을 때 모든키의 입력이 해제 되면 본격적으로 매핑을 시작한다.
*/
// extern void startKeyMapping(void);
/**
키매핑을 종료하고 키보드 상태로 돌아간다.
*/
// extern void stopKeyMapping(void);
// extern void countKeyMappingEnabled(void);
// extern uint8_t isMacroKey(uint8_t xKeyidx);

#endif