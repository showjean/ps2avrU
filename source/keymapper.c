#ifndef KEYMAPPER_C
#define KEYMAPPER_C

#include "timerinclude.h"
#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/eeprom.h>

#include <util/delay.h>
#include <string.h>

// #include "print.h"
#include "keymapper.h"
#include "keymap.h"
#include "keymatrix.h"
#include "keyindex.h"
#include "ps2avru_util.h"
#include "macrobuffer.h"
#include "main.h"
//#include "ps2main.h"
#include "usbmain.h"
#include "ledrender.h"
#include "keydownbuffer.h"
#include "quickswap.h"
//#include "lazyfn.h"
#include "custommacro.h"
#include "dualaction.h"
#include "bootmapper.h"
#include "fncontrol.h"
#include "esctilde.h"
#include "oddebug.h"

#ifndef DISABLE_HARDWARE_MENU
const char str_select_mode[] PROGMEM =  "select mode";

const char str_exit_msg[] PROGMEM =  "good bye";
const char str_space[] PROGMEM = " ";
const char str_colon[] PROGMEM = ":";
const char str_on[] PROGMEM = "on";
const char str_off[] PROGMEM = "off";
const char str_toggle[] PROGMEM = "toggle";
const char str_exit[] PROGMEM = "exit";
const char str_back[] PROGMEM = "back";
const char str_macro[] PROGMEM = "macro";

#ifdef ENABLE_BOOTMAPPER
const char str_bootmapper[] PROGMEM =  "boot mapper";
const char str_start[] PROGMEM =  "start";
#endif

const char str_macro_1[] PROGMEM = "1:select macro index";
const char str_macro_2[] PROGMEM = "2:clear macro";
const char str_macro_9[] PROGMEM = "9:clear all";


#ifndef DISABLE_HARDWARE_KEYMAPPING 
const char str_mapper[] PROGMEM = "key mapper";
const char str_mapper_1[] PROGMEM = "1:change layer";
const char str_mapper_2[] PROGMEM = "2:select key - input keycode";
const char str_mapper_3[] PROGMEM = "3:save and exit";
const char str_mapper_4[] PROGMEM = "4:exit without saving";
const char str_mapper_9[] PROGMEM = "9:reset to default (current layer)";
const char str_choose_layer[] PROGMEM =  "choose layer (1: normal, 2: FN, 3: FN2) current= ";
const char str_choose_key[] PROGMEM = "select key (any key you want) ";
const char str_input_keycode[] PROGMEM = "input keycode (must 3 numbers, clear:000) : ";
const char str_save_end_mapping[] PROGMEM = "save & exit, thank you.";
const char str_cancel_mapping[] PROGMEM = "exit without saving, see you later.";
const char str_reset_mapping[] PROGMEM = "reset to default current layer";
const char str_save_layer[] PROGMEM = "save current layer";
const char str_load_layer[] PROGMEM = "load current layer";
const char str_col_row[] PROGMEM = "col, row : ";
const char str_saving[] PROGMEM = "saving...";
#endif

const char str_prepare_message[] PROGMEM =  "hello";
const char str_input_command[] PROGMEM = ">> ";
const char str_nothing[] PROGMEM = "nothing";

const char str_select_number[] PROGMEM = "input macro index (01~12, cancel: 00, must 2 numbers) : ";
const char str_select_number_to_clear[] PROGMEM = "select index (01~12, cancel: 00) : ";
const char str_input_macro[] PROGMEM = "input key (max 24 keys, stop : press ESC during 1 sec)";
const char str_invalid_number[] PROGMEM = "invalid number, input again.";
const char str_clear_all_macro[] PROGMEM = "clear...";

static uint8_t enabledKeyMappingCount = COUNT_TYPE_NONE;
static int keyMappingCount = 0;
static int keyMappingCountMax = KEY_MAPPING_COUNT_MAX;
static uint8_t _keyMappingOnBoot = 0;
static uint8_t _isKeyMapping = 0;	// 0b00000001 : will start mapping, 0b00000011 : did start mapping
#endif

static uint8_t _macroIndex;
static uint8_t _macroBufferIndex;
static uint8_t _macroPressedBuffer[MACRO_SIZE_MAX_HALF];
static uint8_t _macroInputBuffer[MACRO_SIZE_MAX];

static uint8_t _macroDownCount = 0;
static bool _isQuickMacro = false;
static bool _isQuickMacroStopped;
//static uint8_t _quickMacroStoppedCount = 0;
static void __stopQuickMacro(void);

static uint8_t _step;
#ifndef DISABLE_HARDWARE_MENU
static int _pressedEscapeKeyCount = 0;
static uint8_t _isPressedEscapeKey = 0;
static uint8_t _isTiredEscapeKey = 0;
static uint8_t _wait;			// 매크로 완료 후 실행될 작업 구분;
static uint8_t _mode;
#ifndef DISABLE_HARDWARE_KEYMAPPING
static uint8_t _stepAfterLayerSave;	// save 후 실행될 step 저장;
static uint8_t _col, _row;
static int _editCount;	// 카운트가 1이상이어야만 eeprom에 write 한다.
uint8_t _newKeyMap[ROWS][COLUMNS];
static uint8_t _currentLayer;
static uint8_t _currentLayerAfter;
#endif
static uint8_t _buffer[3];
static uint8_t _bufferIndex;
static uint8_t _isWorkingForEmpty = 0;	// 매크로 버퍼가 모두 소진된 후 진행할 내용이 있는지 확인;
#endif


#ifndef DISABLE_HARDWARE_MENU
// 키매핑에 사용되는 키들 정의;
static uint8_t usingKeys[11] = {
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_ESC	//, KEY_S, KEY_X, KEY_L, KEY_R           
};
#endif

// define functions
void readMacro(uint8_t xMacroIndex);

#ifndef DISABLE_HARDWARE_MENU
void printPrepareString(void);
static void prepareKeyMapping(void);
void prepareKeyMapper(void);

#ifndef	DISABLE_HARDWARE_KEYMAPPING				
void saveCurrentLayerAfter(void);
void printMapperMessageAfter(void);
#endif
void printSelectModeAfter(void);
void clearMacroAllAfter(void);
void clearMacroAtIndexAfter(void);
void printSelectMode(void);

static keymapper_driver_t *_drivers[4];
static uint8_t _driverCount;
static uint8_t _driverIndex;
static uint8_t _driverIndexOffset;

void initKeymapper(void){
	keyMappingCountMax = setDelay(KEY_MAPPING_COUNT_MAX);

#ifndef DISABLE_HARDWARE_KEYMAPPING
	_driverIndexOffset = 2;
#else
	_driverIndexOffset = 1;
#endif

	_driverCount = 0;
}
void addKeymapperDriver(keymapper_driver_t *xDriver){
	_drivers[_driverCount] = xDriver;
	++_driverCount;
}
void setStep(uint8_t xStep){
	_step = xStep;
}
//------------------------------------------------------///

uint8_t isKeyMapping(void){
	return _isKeyMapping & BV(0);	// will start key mapping
}

static void setWillStartKeyMapping(void){
    _isKeyMapping |= BV(0); //set will start mapping
}

void setDeepKeyMapping(void){
	setWillStartKeyMapping();
	_isKeyMapping |= BV(1);	//set doing mapping
}
#endif

uint8_t isDeepKeyMapping(void){

#ifndef DISABLE_HARDWARE_MENU
	return _isKeyMapping & BV(1);	// did start key mapping
#else
	return _isQuickMacro;
#endif
}

#ifndef DISABLE_HARDWARE_MENU
static void applyKeyMapping(uint8_t xModi) {
	static uint8_t prevModifier = 0;

	if(isKeyMapping()) return;

	if(xModi == prevModifier) return;

//	DEBUG_PRINT(("xModi= %d, _isKeyMapping= %d \n", xModi, _isKeyMapping));

	// 약 5초간 입력이 지속되면 키매핑 모드로
	if(xModi == KEYMAPPER_START_KEYS && enabledKeyMappingCount == COUNT_TYPE_NONE){
		keyMappingCount = 0;
		enabledKeyMappingCount = COUNT_TYPE_COUNTING;
	}else{		
		_isKeyMapping = 0;
		enabledKeyMappingCount = COUNT_TYPE_NONE;
	}

	prevModifier = xModi;

}

/*void showP2UMenu(void){
	prepareKeyMapping();
}*/

/**
 진입키가 입력되면 매핑 시작을 준비한다.
*/
static void prepareKeyMapping(void){
	setWillStartKeyMapping();	//set will start mapping

	blinkOnce(50);
	_delay_ms(50);
	blinkOnce(50);
	_delay_ms(50);
	blinkOnce(100);

}
static void startKeyMappingDeep(void)
{
	setDeepKeyMapping();
	printPrepareString();
	prepareKeyMapper();
}

uint8_t isReadyKeyMappingOnBoot(void){
	return _keyMappingOnBoot;
}

// 부팅시 키매핑 시작
void readyKeyMappingOnBoot(void)
{
	_keyMappingOnBoot = 1;
}

void startKeyMappingOnBoot(void)
{

#ifdef ENABLE_BOOTMAPPER	
	if(isBootMapper()) {
		_keyMappingOnBoot = 0;
		return;
	}
#endif

	if(_keyMappingOnBoot == 1)
	{
		prepareKeyMapping();
		_keyMappingOnBoot = 0;
	}
}

/**
매핑 준비가 되었을 때 모든키의 입력이 해제 되면 본격적으로 매핑을 시작한다.
*/
static void startKeyMapping(void){
		// isReleaseAll()로 비교하면 ps/2연결시 마지막 키의 up 판단 전에 매트릭스상 모든 키가 릴리즈 상태여서 마지막 키가 리포트 되지 않는다.
	if(isKeyMapping() && !isDeepKeyMapping() && isReleaseAll()){
		startKeyMappingDeep();
	}
}

/**
키매핑을 종료하고 키보드 상태로 돌아간다.
*/
void stopKeyMapping(void){
	_isKeyMapping = 0;
	_wait = WAIT_NOTHING;
	// DEBUG_PRINT(("stopKeyMapping : _isKeyMapping= %d \n", _isKeyMapping));
	DBG1(0xff, (uchar *)&_isKeyMapping, 1);
}

static void countKeyMappingEnabled(void){	
	if(!_isKeyMapping && enabledKeyMappingCount == COUNT_TYPE_COUNTING && ++keyMappingCount > keyMappingCountMax){
		prepareKeyMapping();			
		enabledKeyMappingCount = COUNT_TYPE_KEYMAPPER;
	}

}
#endif


void enterFrameForMapper(void){

#ifndef DISABLE_HARDWARE_MENU
	applyKeyMapping(getModifierDownBuffer());

	startKeyMapping();

	countKeyMappingEnabled();

	if(isDeepKeyMapping()) {		
		if(_isWorkingForEmpty && !isActiveMacro() && !hasUpdate())
		{
			// DEBUG_PRINT(("_wait : %d \n", _wait));
			if(_wait == WAIT_SELECT_MODE){
				printSelectModeAfter();
			}
#ifndef	DISABLE_HARDWARE_KEYMAPPING
			else if(_wait == WAIT_SAVE){
                saveCurrentLayerAfter();
            }else if(_wait == WAIT_KEYMAPPER){
				printMapperMessageAfter();
			}
#endif
			else if(_wait == WAIT_CLEAR_MACRO){
				clearMacroAtIndexAfter();
			}else if(_wait == WAIT_CLEAR_ALL_MACRO){
				clearMacroAllAfter();
			}else{
				_isWorkingForEmpty = 0;
			}
			_wait = WAIT_NOTHING;
		}
		
		if(_isPressedEscapeKey && ++_pressedEscapeKeyCount > 1000){
			_isTiredEscapeKey = 1;
			putKeyindex(KEY_ESC, 255, 255, 0);	// esc가 up 된것으로 알림;
		}
	}
#else
	// for quick macro save
	if(_isQuickMacroStopped && isReleaseAll() && !isActiveMacro()) // && ++_quickMacroStoppedCount > 10)
	{
//	    _quickMacroStoppedCount = 0;
	    __stopQuickMacro();
	}
#endif
}


#ifndef DISABLE_HARDWARE_MENU
void printEnter(void)
{
    pushMacroKeyIndex(KEY_ENTER);
    pushMacroKeyIndex(KEY_ENTER);
}

void printStringAndFlash(const char *xStr, const char *xfStr){
	printString(xStr);
	printStringFromFlash(xfStr);
}
#endif

static void pushCharacter(char *xStr)
{
    // 버퍼에 쌓아두고 모두 출력되기를 기다린다.
    macro_key_t key = charToKey(xStr[0]);
    if(key.mode){
        pushMacroKeyIndex(KEY_LSHIFT);
    }
    pushMacroKeyIndex(key.keyindex);    // 같은 코드를 2개 입력한다. 첫번째는 press, 두번재는 release
    pushMacroKeyIndex(key.keyindex);
    // DEBUG_PRINT(("pushCharacter char %c : %d \n", xStr[0], key.keyindex));
    if(key.mode){
        pushMacroKeyIndex(KEY_LSHIFT);
    }
   
}

void printString(const char *xString)
{
	char c;
	char gChar[1];
	uint8_t i;
	// DEBUG_PRINT(("printString len : %d string : %s \n", strlen(xString), xString));
    for (i = 0; i < strlen(xString); i++) {
        c = xString[i];
        // DEBUG_PRINT(("i = %d, %c ", i, c));
        sprintf(gChar, "%c", c);
    	pushCharacter(gChar);
    	
    }
}
#ifndef DISABLE_HARDWARE_MENU
void printStringFromFlashWithEnter(const char *str){
	printStringFromFlash(str);
	printEnter();
}
void printStringFromFlash(const char *str) 
{ 	
	char c;
	char gChar[1];
	
  	if(!str) return;
  	while((c = pgm_read_byte(str++))){ 
    	sprintf(gChar, "%c", c);
    	pushCharacter(gChar);
    } 
}

static void printPrompt(void)
{
	// char gStr[1];

	// 모든 프린트 작업중에는 입력을 받지 않도록;
	_isWorkingForEmpty = 1;
	switch(_step){

#ifndef DISABLE_HARDWARE_KEYMAPPING
		case STEP_CHOOSE_LAYER:		
			printStringFromFlash(str_choose_layer);
			printString(toString(_currentLayer+1));
			printEnter();
		break;
		case STEP_CHOOSE_KEY:
			printStringFromFlashWithEnter(str_choose_key);
		break;
		case STEP_INPUT_KEYCODE:
			printStringFromFlash(str_input_keycode);
		break;
		case STEP_SAVE_END_MAPPING:
			printStringFromFlashWithEnter(str_save_end_mapping);
			_step = STEP_NOTHING;
		break;
		case STEP_CANCEL_MAPPING:
			printStringFromFlashWithEnter(str_cancel_mapping);
			_step = STEP_NOTHING;
		break;
		case STEP_RESET_MAPPING:
			_isWorkingForEmpty = 0;
		break;
#endif
		case STEP_INPUT_COMMAND:
		case STEP_SELECT_MODE:
			printStringFromFlash(str_input_command);
		break;
		case STEP_SELECT_INDEX:
			printStringFromFlash(str_select_number);
		break;
		case STEP_CLEAR_SELECT_INDEX:
			printStringFromFlash(str_select_number_to_clear);
		break;
		break;
		case STEP_INPUT_MACRO:
			printStringFromFlashWithEnter(str_input_macro);
		break;
#ifdef ENABLE_BOOTMAPPER
		case STEP_BOOT_MAPPER:
			printStringFromFlashWithEnter(str_bootmapper);
			_step = STEP_NOTHING;
			setToBootMapper(true);
		break;
#endif
		case STEP_EXIT:
			stopKeyMapping();
			printStringFromFlashWithEnter(str_exit_msg);
			_step = STEP_NOTHING;
		break;
		case STEP_BACK:
			printSelectMode();
		break;
		case STEP_NOTHING:
		break;
	}

	if(_step != STEP_NOTHING) _delay_ms(100);
}

void printSelectModeAfter(void){

	printStringAndFlash(toString(SEL_EXIT), str_colon);
	printStringFromFlashWithEnter(str_exit);
#ifdef ENABLE_BOOTMAPPER
	printStringAndFlash(toString(SEL_BOOT_MAPPER), str_colon);
	printStringFromFlashWithEnter(str_bootmapper);
#endif
	
	_step = STEP_SELECT_MODE;
	printPrompt();
}

void printSelectMode(void){	
	printEnter();

	printStringFromFlashWithEnter(str_select_mode);
	printStringAndFlash(toString(SEL_MACRO), str_colon);
	printStringFromFlashWithEnter(str_macro);

#ifndef	DISABLE_HARDWARE_KEYMAPPING
	printStringAndFlash(toString(SEL_MAPPING), str_colon);
	printStringFromFlashWithEnter(str_mapper);
#endif

	uint8_t i;
	for (i = 0; i < _driverCount; ++i)
	{
		printStringAndFlash(toString(i+1+_driverIndexOffset), str_colon);
		(*_drivers[i]->printMenu)();
		printEnter(); 
	}
	
	// 256 바이트를 넘어 모두 출력하지 못하므로 잘라서 실행;
	_isWorkingForEmpty = 1;
	_wait = WAIT_SELECT_MODE;
	_step = STEP_NOTHING;
}

#ifndef	DISABLE_HARDWARE_KEYMAPPING				
void printMapperMessageAfter(void)
{
	printEnter();
	printStringFromFlashWithEnter(str_mapper_3);
	printStringFromFlashWithEnter(str_mapper_4);
	printStringAndFlash(toString(CMD_BACK), str_colon);
	printStringFromFlashWithEnter(str_back);
	printStringFromFlashWithEnter(str_mapper_9);
	
	_step = STEP_INPUT_COMMAND;
	printPrompt();
}
static void printMapperMessage(void)
{
	printEnter();
	printStringFromFlashWithEnter(str_mapper);
	printStringFromFlashWithEnter(str_mapper_1);
	printStringFromFlash(str_mapper_2);

	// 256 바이트를 넘어 모두 출력하지 못하므로 잘라서 실행;
	_isWorkingForEmpty = 1;
	_wait = WAIT_KEYMAPPER;
	_step = STEP_NOTHING;
}
#endif

void printMacroMessage(void){
	printEnter();
	printStringFromFlashWithEnter(str_macro);
	printStringFromFlashWithEnter(str_macro_1);
	printStringFromFlash(str_macro_2);
	printEnter();
	printStringAndFlash(toString(CMD_EXIT_MACRO), str_colon);
	printStringFromFlashWithEnter(str_exit);
	printStringAndFlash(toString(CMD_BACK), str_colon);
	printStringFromFlashWithEnter(str_back);
	printStringFromFlashWithEnter(str_macro_9);

	_step = STEP_INPUT_COMMAND;
}


void printPrepareString(void){
	printStringFromFlashWithEnter(str_prepare_message);
}

#ifndef DISABLE_HARDWARE_KEYMAPPING
void loadCurrentLayer(void)
{	
	/*uint16_t gAddress;
	uint8_t k, j;
	for(k = 0; k < ROWS; ++k){
		for (j = 0; j < COLUMNS; ++j)
		{
			gAddress = EEPROM_MAPPING + (k * COLUMNS + j) + (ROWS * COLUMNS * _currentLayer);	// key
			_newKeyMap[k][j] = eeprom_read_byte((uint8_t *)gAddress);
		}
	}*/

    eeprom_read_block(&_newKeyMap, (uint8_t *)(EEPROM_MAPPING + (ROWS * COLUMNS * _currentLayer)), ROWS * COLUMNS);
	_editCount = 0;
}
#endif

void prepareKeyMapper(void)
{
	memset(_buffer, 0, 3);
	_bufferIndex = 0;
#ifndef DISABLE_HARDWARE_KEYMAPPING
	_currentLayer = 0;
#endif
	_mode = 0;

	printSelectMode();
}

#ifndef DISABLE_HARDWARE_KEYMAPPING
void saveCurrentLayerAfter(void)
{
	/*uint8_t gKeyindex;
	uint16_t gAddress;
	uint8_t k, j;
	for(k = 0; k < ROWS; ++k){
		for (j = 0; j < COLUMNS; ++j)
		{
			gKeyindex = _newKeyMap[k][j];	// value
			gAddress = EEPROM_MAPPING + (k * COLUMNS + j) + (ROWS * COLUMNS * _currentLayer);	// key
			eeprom_update_byte((uint8_t *)gAddress, gKeyindex);
		}
	}*/

	eeprom_update_block(&_newKeyMap, (uint8_t *)(EEPROM_MAPPING + (ROWS * COLUMNS * _currentLayer)), ROWS * COLUMNS);

	_editCount = 0;

	if(_stepAfterLayerSave == STEP_SAVE_END_MAPPING)
	{
		_step = _stepAfterLayerSave;		
		stopKeyMapping();
	}else if(_stepAfterLayerSave == STEP_CHOOSE_LAYER)
	{	
		printEnter();
		printStringFromFlashWithEnter(str_load_layer);
		_step = STEP_INPUT_COMMAND;
		_currentLayer = _currentLayerAfter;			
		loadCurrentLayer();
	}else{

	}
	printPrompt();
}
void saveCurrentLayer(void)
{	
	if(_editCount == 0) {
		stopKeyMapping();
		return;
	}

	printStringFromFlashWithEnter(str_saving);

	_isWorkingForEmpty = 1;

	_wait = WAIT_SAVE;
	_stepAfterLayerSave = _step;
	_step = STEP_NOTHING;
}
void resetCurrentLayer(void)
{
    printStringFromFlashWithEnter(str_reset_mapping);
    // 현재 레이어 기본값으로 복구;
    uint8_t k, j;
    for(k = 0; k < ROWS; ++k){
        for (j = 0; j < COLUMNS; ++j)
        {
            _newKeyMap[k][j] = 0;
        }
    }
    _editCount = 1;

}
#endif
#endif


bool isMacroKey(uint8_t xKeyidx){
	if(xKeyidx >= KEY_CST_MAC1 && xKeyidx <= KEY_MAC12){
		return true;
	}
	return false;

}

bool isEepromMacroKey(uint8_t xKeyidx){
	if(xKeyidx >= KEY_MAC1 && xKeyidx <= KEY_MAC12){
		return true;
	}
	return false;

}

// 매크로 적용됐으면 1, 아니면 0 반환;
uint8_t applyMacro(uint8_t xKeyidx){
#ifndef DISABLE_HARDWARE_MENU
	if(isKeyMapping()) return 0;	// 키매핑이 아닐때만 매크로 적용;
#endif

	uint8_t gMacroIndex;
	// DEBUG_PRINT(("applyMacro  xKeyidx: %d isMacroKey: %d \n", xKeyidx, isMacroKey(xKeyidx)));
	if(isMacroKey(xKeyidx)){		
		if(!isActiveMacro()){
			if(xKeyidx >= KEY_MAC1){	// eeprom macro
				gMacroIndex = xKeyidx - KEY_MAC1;			
				uint8_t gKeyidx = eeprom_read_byte((uint8_t *)(EEPROM_MACRO+(MACRO_SIZE_MAX * gMacroIndex)));
				if(gKeyidx > 0 && gKeyidx < 255){
					clearMacroPressedBuffer();
					readMacro(gMacroIndex);
				}
			}else{	// custom macro
				gMacroIndex = xKeyidx - KEY_CST_MAC1;		
				if(hasCustomMacroAt(gMacroIndex)){
					clearMacroPressedBuffer();
					readCustomMacroAt(gMacroIndex);
				}
			}

			return 1;
		}else if(isRepeat()){
			clearRepeat();
			stopRepeat();

			return 1;
		}
	}
	return 0;
}

void readMacro(uint8_t xMacroIndex){
	if(xMacroIndex >= MACRO_NUM) return;

	uint16_t gAddress;
	uint8_t gKeyindex;
	uint8_t k;
	for(k = 0; k < MACRO_SIZE_MAX; ++k){
		gAddress = EEPROM_MACRO + (k) + (MACRO_SIZE_MAX * xMacroIndex);	// key
		gKeyindex = eeprom_read_byte((uint8_t *)gAddress);
		if(gKeyindex > 0 && gKeyindex < 255){
			pushMacroKeyIndex(gKeyindex);
		}
	}
}

void saveMacro(void){
	if(_macroIndex >= MACRO_NUM) return;

	eeprom_update_block(&_macroInputBuffer, (uint8_t *)(EEPROM_MACRO + (MACRO_SIZE_MAX * _macroIndex)), MACRO_SIZE_MAX);
	_macroIndex = 255;
}

#ifndef DISABLE_HARDWARE_MENU
void clearMacroAllAfter(void){
//	uint16_t gAddress;
	int k;
	for(k = 0; k < MACRO_SIZE_MAX * MACRO_NUM; ++k){
//		gAddress = EEPROM_MACRO + (k);	// key
		eeprom_update_byte((uint8_t *)(EEPROM_MACRO + (k)), 0);
	}

//	eeprom_update_block(&_macroInputBuffer, (uint8_t *)(EEPROM_MACRO), MACRO_SIZE_MAX * MACRO_NUM);

	_step = STEP_INPUT_COMMAND;
	printPrompt();
}

void clearMacroAll(void)
{
	printStringFromFlashWithEnter(str_clear_all_macro);

	_isWorkingForEmpty = 1;

	_wait = WAIT_CLEAR_ALL_MACRO;
	_step = STEP_NOTHING;
}

void clearMacroAtIndexAfter(void){
//	uint16_t gAddress;
	int k;
	int gLen = MACRO_SIZE_MAX;
	for(k = 0; k < gLen; ++k){
//		gAddress = EEPROM_MACRO + (k) + (MACRO_SIZE_MAX * _macroIndex);	// key
		eeprom_update_byte((uint8_t *)(EEPROM_MACRO + (k) + (MACRO_SIZE_MAX * _macroIndex)), 0);
	}
	_step = STEP_INPUT_COMMAND;
	printPrompt();
}

void clearMacroAtIndex(void){	
	printStringFromFlash(str_macro);
	printStringFromFlash(str_space);
	printStringAndFlash(toString(_macroIndex+1), str_space);
	printStringFromFlashWithEnter(str_clear_all_macro);

	_isWorkingForEmpty = 1;

	_wait = WAIT_CLEAR_MACRO;
	_step = STEP_NOTHING;
}
#endif

bool isMacroInput(void){
	if(_step == STEP_INPUT_MACRO){
		return true;
	}else{
		return false;
	}
}

static void resetMacroInput(void){	
	memset(_macroInputBuffer, 0, MACRO_SIZE_MAX);
	_macroBufferIndex = 0;
	memset(_macroPressedBuffer, 0, MACRO_SIZE_MAX);
	_macroDownCount = 0;
}

bool isQuickMacro(void){
	return _isQuickMacro;
}

void startQuickMacro(uint8_t xMacroIndex){
	resetMacroInput();
	_macroIndex = xMacroIndex;
	_isQuickMacro = true;
	_isQuickMacroStopped = false;
#ifndef DISABLE_HARDWARE_MENU
	_step = STEP_INPUT_MACRO;
	setDeepKeyMapping();
#endif

	blinkOnce(200);
	_delay_ms(100);
	blinkOnce(100);
}

static void __stopQuickMacro(void){

//  DBG1(0xef, (uchar *)&_macroBufferIndex, 1);
    _isQuickMacroStopped = false;
    saveMacro();
    _macroIndex = 255;
#ifndef DISABLE_HARDWARE_MENU
    _step = STEP_NOTHING;
    stopKeyMapping();
#endif

    blinkOnce(100);
}

void stopQuickMacro(void){
    _isQuickMacro = false;
    _isQuickMacroStopped = true;
}

static void stopMacroInput(void){
//	DBG1(0xee, (uchar *)&_macroBufferIndex, 1);
	if(_isQuickMacro){
	    stopQuickMacro();
	}
#ifndef DISABLE_HARDWARE_MENU
	else{
		saveMacro();
		_step = STEP_INPUT_COMMAND;
		printEnter();
		printPrompt();
	}
#endif
}

static uint8_t putMacro(uint8_t xKeyidx, uint8_t xIsDown){
    int gIdx;

	if(xKeyidx >= KEY_MAX) return KEY_NONE;		// 매크로 입력시 키값으로 변환할 수 없는 특수 키들은 중단;

#ifndef DISABLE_HARDWARE_MENU
	if(_isTiredEscapeKey){
		_isPressedEscapeKey = 0;
		_isTiredEscapeKey = 0;

		pushMacroKeyIndex(xKeyidx);
		// 또는 ESC를 1초이상 누르면 종료;
		_macroInputBuffer[_macroBufferIndex-1] = 0;	// 마지막 esc 눌림 제거;

		stopMacroInput();

		return KEY_NONE;
	}
#endif

	if(xIsDown){
		if(_macroDownCount >= MACRO_SIZE_MAX_HALF){	// 매크로 크기의 절반이 넘은 키 다운은 제외 시킨다. 그래야 나머지 공간에 up 데이터를 넣을 수 있으므로.
			return KEY_NONE;
		}
		++_macroDownCount;
	    DBG1(0x07, (uchar *)&xKeyidx, 1);  
	   
	    append(_macroPressedBuffer, xKeyidx);
	    
	}else{
	    gIdx = findIndex(_macroPressedBuffer, xKeyidx);
	    // 릴리즈시에는 프레스 버퍼에 있는 녀석만 처리; 버퍼에 없는 녀석은 16키 이후의 키이므로 제외;

	    if(gIdx == -1){
	    	return KEY_NONE;
	    }
	    delete(_macroPressedBuffer, gIdx);
	    DBG1(0x08, (uchar *)&xKeyidx, 1);  
	}

	_macroInputBuffer[_macroBufferIndex] = xKeyidx;
	++_macroBufferIndex;

	DBG1(0x09, (uchar *)&_macroInputBuffer, strlen((char *)_macroInputBuffer));  

//	pushMacroKeyIndex(xKeyidx);

	// MACRO_SIZE_MAX개를 채웠다면 종료;
	if(_macroBufferIndex >= MACRO_SIZE_MAX){
		// _macroIndex 위치에 저장;
	
		stopMacroInput();

		return xKeyidx;
	}

#ifndef DISABLE_HARDWARE_MENU
	if(xIsDown && xKeyidx == KEY_ESC){
		//esc 눌렸음;
		_isPressedEscapeKey = 1;
		_pressedEscapeKeyCount = 0;
	}else{
		_isPressedEscapeKey = 0;
		_isTiredEscapeKey = 0;
	}
#endif

	return xKeyidx;
}

uint8_t putKeyindex(uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, uint8_t xIsDown)
{
#ifndef DISABLE_HARDWARE_MENU
	uint8_t cmd;
#ifndef DISABLE_HARDWARE_KEYMAPPING
	uint8_t gLayer;
#endif
	int gKeyCode;
	int gKeyIndex; 
    int gIdx;

    DBG1(0x01, (uchar *)&_isWorkingForEmpty, 1);
	// 매크로 실행중에는 입력을 받지 않는다.
	if(_isWorkingForEmpty) return KEY_NONE;

#endif
    xKeyidx = getDualActionDownKeyIndexWhenIsCompounded(xKeyidx, false);

//    DBG1(0x11, (uchar *)&xKeyidx, 1);
#ifndef DISABLE_HARDWARE_MENU
	// 매핑 중에는 키 업만 실행 시킨다.
	if(!isMacroInput() && xIsDown) return KEY_NONE;	// 매크로 일 경우에만 다운 키 실행;
	if(isMacroInput()){
#endif
		return	putMacro(xKeyidx, xIsDown);
#ifndef DISABLE_HARDWARE_MENU
	}

	gKeyIndex = findIndex(usingKeys, xKeyidx);

	// 입력되는 키코드 값은 usb/ps2 모두 동일하다.
	if(_step == STEP_INPUT_COMMAND || _step == STEP_SELECT_MODE){
		if(gKeyIndex >= 0 && gKeyIndex < 10){	// 0~9
			_buffer[_bufferIndex] = gKeyIndex;
		
			printString(toString(gKeyIndex));
			printEnter();

			// 커맨드에 따라서 스텝 지정;
			cmd = _buffer[0];

			if(_step == STEP_SELECT_MODE){
				if(cmd == SEL_MACRO){
					_mode = SEL_MACRO;
					printMacroMessage();
				}
#ifndef	DISABLE_HARDWARE_KEYMAPPING
				else if(cmd == SEL_MAPPING){
					_mode = SEL_MAPPING;
					loadCurrentLayer();
					printMapperMessage();
				}
#endif
				else if(cmd == SEL_EXIT){
					_step = STEP_EXIT;
#ifdef ENABLE_BOOTMAPPER
				}else if(cmd == SEL_BOOT_MAPPER){
					_mode = SEL_BOOT_MAPPER;
					_step = STEP_BOOT_MAPPER;
					stopKeyMapping();
#endif
				}else{
					gIdx = cmd-1-_driverIndexOffset;
					if(gIdx < _driverCount){
						_mode = SEL_OPTIONS;
						_driverIndex = gIdx;
						(*_drivers[_driverIndex]->printContents)();
						_step = STEP_INPUT_COMMAND;
					}

				}
				
			}else if(_step == STEP_INPUT_COMMAND){
				if(cmd == CMD_BACK){
					_step = STEP_BACK;
				}
#ifndef DISABLE_HARDWARE_KEYMAPPING
				if(_mode == SEL_MAPPING){
					if(cmd == CMD_CHOOSE_LAYER){
						_step = STEP_CHOOSE_LAYER;
					}else if(cmd == CMD_SELECT_KEY){
						_step = STEP_CHOOSE_KEY;
					}else if(cmd == CMD_SAVE_AND_EXIT){
						_step = STEP_SAVE_END_MAPPING;
						// 키코드를 저장하고 종료;
						saveCurrentLayer();
					}else if(cmd == CMD_CANCEL_WITHOUT_SAVE){
						_step = STEP_CANCEL_MAPPING;
						// 이전 상태를 유지한채 종료;
						stopKeyMapping();		
					}else if(cmd == CMD_RESET_TO_DEFAULT){
						// _step = STEP_RESET_MAPPING;
						resetCurrentLayer();
						_step = STEP_INPUT_COMMAND;
					}
				}else
#endif
				    if(_mode == SEL_MACRO){
					if(cmd == CMD_SELECT_INDEX){
						_step = STEP_SELECT_INDEX;
					}else if(cmd == CMD_CLEAR_MACRO){
						_step = STEP_CLEAR_SELECT_INDEX;
					}else if(cmd == CMD_EXIT_MACRO){
						_step = STEP_EXIT;
					}else if(cmd == CMD_CLEAR_ALL_MACRO){
						_step = STEP_CLEAR_ALL_MACRO;
						clearMacroAll();
					}
				}else if(_mode == SEL_OPTIONS){
					(*_drivers[_driverIndex]->putKeyindex)(cmd, xKeyidx, xCol, xRow, xIsDown);
				}
			}
			_bufferIndex = 0;

			printPrompt();
		}
		return KEY_NONE;

	}else if(_step == STEP_CHOOSE_KEY){
		// do not anything;
	}else if(gKeyIndex >= 0 && gKeyIndex < 10){	// 0~9

		_buffer[_bufferIndex] = gKeyIndex;
		++_bufferIndex;

		printString(toString(gKeyIndex));		

	}else{
		// DEBUG_PRINT(("bad command \n"));
		return KEY_NONE;
	}
	
	// key mapper
	if((gKeyIndex >= 0 && gKeyIndex < 10) || _step == STEP_CHOOSE_KEY)
	{		
		switch(_step){
			case STEP_SELECT_INDEX:
				if(_bufferIndex == 2)
				{
					gKeyCode = (_buffer[0] * 10);
					if(gKeyCode >= 0){
						gKeyCode = gKeyCode + _buffer[1];
					}

					if(gKeyCode > 0 && gKeyCode <= MACRO_NUM){
						_macroIndex = gKeyCode - 1;
						_step = STEP_INPUT_MACRO;
						resetMacroInput();
					}else if(gKeyCode == 0){
						_step = STEP_INPUT_COMMAND;
					}else{
						printEnter();
						printStringFromFlash(str_invalid_number);
					}

					memset(_buffer, 0, 3);
					_bufferIndex = 0;
					printEnter();
				}else{
					_delay_ms(100);
					return KEY_NONE;
				}
			break;	
			case STEP_CLEAR_SELECT_INDEX:
				if(_bufferIndex == 2)
				{
					gKeyCode = (_buffer[0] * 10);
					if(gKeyCode >= 0){
						gKeyCode = gKeyCode + _buffer[1];
					}

					if(gKeyCode > 0 && gKeyCode <= MACRO_NUM){
						_macroIndex = gKeyCode - 1;
						_step = STEP_INPUT_COMMAND;
						printEnter();
						clearMacroAtIndex();
					}else if(gKeyCode == 0){
						_step = STEP_INPUT_COMMAND;
						printEnter();
					}else{
						printEnter();
						printStringFromFlashWithEnter(str_invalid_number);
					}

					memset(_buffer, 0, 3);
					_bufferIndex = 0;
				}else{
					_delay_ms(100);
					return KEY_NONE;
				}
			break;	
#ifndef DISABLE_HARDWARE_KEYMAPPING
			case STEP_CHOOSE_LAYER:
			
				gLayer = _buffer[0] - 1;
				if(_currentLayer < 3 && _currentLayer != gLayer && (gLayer >= 0 && gLayer < 3)){
					if(_editCount > 0){
						printEnter();
						printStringFromFlashWithEnter(str_save_layer);
						saveCurrentLayer();
						_currentLayerAfter = gLayer;
					}else{
						printEnter();
						printStringFromFlashWithEnter(str_load_layer);
						_step = STEP_INPUT_COMMAND;
						_currentLayer = gLayer;						
						loadCurrentLayer();
					}
				}else{
					printEnter();	
					_step = STEP_INPUT_COMMAND;
				}

				_bufferIndex = 0;
			break;
			case STEP_CHOOSE_KEY:
			// 키를 누르면 col, row 값을 저장한다.
				_col = xCol;
				_row = xRow;
				printStringFromFlash(str_col_row);				
				printString(toString(_col+1));
				printString(", ");
				printString(toString(_row+1));	
				printString(" (layer=");
				printString(toString(_currentLayer+1));
				printString(" default=");
				gKeyCode = getDefaultKeyindex(_currentLayer, _row, _col);
				printString(toString(gKeyCode));
				printString(" mapping=");
				gKeyCode = _newKeyMap[_row][_col];
				if(gKeyCode == 0xFF) gKeyCode = 0;
				printString(toString(gKeyCode));
				printString(")");
				printEnter();
				
				_step = STEP_INPUT_KEYCODE;
				
				_bufferIndex = 0;
			break;
			case STEP_INPUT_KEYCODE:
			// 키코드 3자리가 모두 입력되면 _newKeyMap의 layer, col, row에 저장한다.
				if(_bufferIndex == 3)
				{
					gKeyCode = (_buffer[0] * 100);
					if(gKeyCode >= 0){
						gKeyCode = gKeyCode + (_buffer[1] * 10);
					}
					if(gKeyCode >= 0){
						gKeyCode = gKeyCode + _buffer[2];
					}

					if(gKeyCode < 0xFF){
						_newKeyMap[_row][_col] = (uint8_t)gKeyCode;
						_step = STEP_INPUT_COMMAND;
						_editCount++;
					}else{
						// 휴효한 키코드가 아닙니다. 다시 입력하세요./////////////////////////////////////////////////
						printEnter();
						printStringFromFlash(str_invalid_number);
					}

					memset(_buffer, 0, 3);
					_bufferIndex = 0;
					printEnter();
				}else{
					_delay_ms(100);
					return KEY_NONE;
				}

			break;
#endif
		}
		printPrompt();

	}
#endif

    return KEY_NONE;
}

#endif
