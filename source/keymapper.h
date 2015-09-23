#ifndef KEYMAPPER_H
#define KEYMAPPER_H

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

#ifdef DEBUG_KEYMAPPER_H
	#define KEY_MAPPING_COUNT_MAX 500
#else
	#define KEY_MAPPING_COUNT_MAX 2500
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
#define STEP_INPUT_MACRO 11
#define STEP_CLEAR_ALL_MACRO 12
#define STEP_CLEAR_SELECT_INDEX 13
// exit
#define STEP_EXIT 100
#define STEP_BACK 110
#ifdef ENABLE_BOOTMAPPER
#define STEP_BOOT_MAPPER 120
#endif

// wait
#define WAIT_NOTHING 0
#define WAIT_KEYMAPPER 10
#define WAIT_SAVE 11
#define WAIT_CLEAR_ALL_MACRO 12
#define WAIT_CLEAR_MACRO 13
#define WAIT_SELECT_MODE 14

// mode select
#define SEL_MACRO 1
#define SEL_MAPPING 2
#define SEL_EXIT 7
#define SEL_BOOT_MAPPER 9
#define SEL_OPTIONS 99

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

// back
#define CMD_BACK 6

#define COUNT_TYPE_NONE		 	0
#define COUNT_TYPE_COUNTING 	1
#define COUNT_TYPE_KEYMAPPER 	2

#define KEYMAPPER_START_KEYS	0x27	// KEY_LCTRL + KEY_LSHIFT + KEY_LALT + KEY_RSHIFT


extern const char PROGMEM str_select_mode[];
extern const char PROGMEM str_select_mode1[];
extern const char PROGMEM str_select_mode2[];
extern const char PROGMEM str_select_mode_exit[];
extern const char PROGMEM str_select_mode_bootmapper[];

extern const char PROGMEM str_exit_msg[];
extern const char PROGMEM str_boot_mapper[];

extern const char PROGMEM str_macro_message[];
extern const char PROGMEM str_macro_1[];
extern const char PROGMEM str_macro_2[];
extern const char PROGMEM str_exit[];
extern const char PROGMEM str_macro_9[];

extern const char PROGMEM str_toggle[];

extern const char PROGMEM str_back[];

#ifndef DISABLE_HARDWARE_KEYMAPPING 
extern const char PROGMEM str_mapper_message[];
extern const char PROGMEM str_mapper_1[];
extern const char PROGMEM str_mapper_2[];
extern const char PROGMEM str_mapper_3[];
extern const char PROGMEM str_mapper_4[];
extern const char PROGMEM str_mapper_9[];
#endif

extern const char PROGMEM str_prepare_message[];
extern const char PROGMEM str_choose_layer[];
extern const char PROGMEM str_choose_key[];
extern const char PROGMEM str_input_keycode[];
extern const char PROGMEM str_save_end_mapping[];
extern const char PROGMEM str_cancel_mapping[];
extern const char PROGMEM str_reset_mapping[];
extern const char PROGMEM str_input_command[];
extern const char PROGMEM str_nothing[];
extern const char PROGMEM str_save_layer[];
extern const char PROGMEM str_load_layer[];
extern const char PROGMEM str_saving[];
extern const char PROGMEM str_col_row[];

extern const char PROGMEM str_select_number[];
extern const char PROGMEM str_select_number_to_clear[];
extern const char PROGMEM str_input_macro[];
extern const char PROGMEM str_invalid_number[];
extern const char PROGMEM str_clear_all_macro[];

extern const char PROGMEM str_space[];
extern const char PROGMEM str_macro[];
extern const char PROGMEM str_colon[];
extern const char PROGMEM str_on[];
extern const char PROGMEM str_off[];

typedef struct {
    void (*printMenu)(void);
    void (*printContents)(void);
    void (*putKeyindex)(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
} keymapper_driver_t;
void addKeymapperDriver(keymapper_driver_t *xDriver);
void printStringFromFlashWithEnter(const char *str);
void printStringFromFlash(const char *str);
void printString(const char *xString);
void stopKeyMapping(void);
void printEnter(void);
void setStep(uint8_t xStep);

void initKeymapper(void);

uint8_t isKeyMapping(void);
uint8_t isDeepKeyMapping(void);
bool isMacroKey(uint8_t xKeyidx);
bool isEepromMacroKey(uint8_t xKeyidx);

//void showP2UMenu(void);

// 부팅시 키 매핑 시작
void readyKeyMappingOnBoot(void);
void startKeyMappingOnBoot(void);

// event loop
void enterFrameForMapper(void);

// 매크로 등록중인지 확인;
bool isMacroInput(void);
// 매크로 적용됐으면 1, 아니면 0 반환;
uint8_t applyMacro(uint8_t xKeyidx);

void putKeyindex(uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, uint8_t xIsDown);

bool isQuickMacro(void);
void startQuickMacro(uint8_t xMacroIndex);
void stopQuickMacro(void);


#endif
