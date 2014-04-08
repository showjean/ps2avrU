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

#include "print.h"
#include "keymapper.h"
#include "keymap.h"
#include "keymatrix.h"
#include "keyindex.h"
#include "ps2avru_util.h"
#include "macrobuffer.h"
#include "main.h"
#include "ps2main.h"
#include "usbmain.h"
#include "ledrender.h"
#include "keydownbuffer.h"
#include "quickswap.h"
#include "lazyfn.h"
#include "custommacro.h"
#include "dualaction.h"
#include "smartkey.h"
#include "bootmapper.h"
#include "fncontrol.h"
#include "oddebug.h"


const char str_select_mode[] PROGMEM =  "select mode";
const char str_select_mode1[] PROGMEM =  "key mapping";
const char str_select_mode2[] PROGMEM =  "macro";
const char str_select_mode_exit[] PROGMEM =  "exit";
const char str_select_mode_bootmapper[] PROGMEM =  "boot mapper";

const char str_exit_msg[] PROGMEM =  "good bye~";
const char str_boot_mapper[] PROGMEM =  "boot mapper start!";

const char str_macro_message[] PROGMEM = "Macro";
const char str_macro_1[] PROGMEM = "1:Select Macro Index";
const char str_macro_2[] PROGMEM = "2:Clear Macro";
const char str_macro_9[] PROGMEM = "9:Clear All";

const char str_toggle[] PROGMEM = "toggle";
const char str_exit[] PROGMEM = "Exit";
const char str_back[] PROGMEM = "Back";

#ifndef DISABLE_HARDWARE_KEYMAPPING 
const char str_mapper_message[] PROGMEM = "Key Mapper";
const char str_mapper_1[] PROGMEM = "1:Change Layer";
const char str_mapper_2[] PROGMEM = "2:Select Key - input keycode";
const char str_mapper_3[] PROGMEM = "3:Save and Exit";
const char str_mapper_4[] PROGMEM = "4:Exit without Saving";
const char str_mapper_9[] PROGMEM = "9:Reset to Default (Current Layer)";
#endif

const char str_prepare_message[] PROGMEM =  "ps2avrU";
const char str_choose_layer[] PROGMEM =  "choose layer (1: normal, 2: FN, 3: FN2) current= ";
const char str_choose_key[] PROGMEM = "select key (any key you want) ";
const char str_input_keycode[] PROGMEM = "input keycode (must 3 numbers, clear:000) : ";
const char str_save_end_mapping[] PROGMEM = "Save & Exit, thank you.";
const char str_cancel_mapping[] PROGMEM = "Exit without Saving, see you later.";
const char str_reset_mapping[] PROGMEM = "Reset to default current layer";
const char str_input_command[] PROGMEM = ">> ";//"Input Command Number: ";
const char str_nothing[] PROGMEM = "nothing";
const char str_save_layer[] PROGMEM = "Save current layer";
const char str_load_layer[] PROGMEM = "Load current layer";
const char str_saving[] PROGMEM = "saving...";
const char str_col_row[] PROGMEM = "col, row : ";

const char str_select_number[] PROGMEM = "input macro index (01~12, cancel: 00, must 2 numbers) : ";
const char str_select_number_to_clear[] PROGMEM = "select index (01~12, cancel: 00) : ";
const char str_input_macro[] PROGMEM = "input key (max 24 keys, stop : press ESC during 1 sec)";
const char str_invalid_number[] PROGMEM = "invalid number, input again.";
const char str_clear_all_macro[] PROGMEM = "clear...";

const char str_space[] PROGMEM = " ";
const char str_macro[] PROGMEM = "macro";
const char str_colon[] PROGMEM = ":";
const char str_on[] PROGMEM = "on";
const char str_off[] PROGMEM = "off";


static uint8_t _isKeyMapping = 0;	// 0b00000001 : will start mapping, 0b00000011 : did start mapping
static uint8_t enabledKeyMappingCount = COUNT_TYPE_NONE;
static int keyMappingCount = 0;
static int keyMappingCountMax = KEY_MAPPING_COUNT_MAX;
static uint8_t _keyMappingOnBoot = 0;

uint8_t _macroIndex;
uint8_t _macroBufferIndex;
uint8_t _macroPressedBuffer[MACRO_SIZE_MAX_HALF];
uint8_t _macroInputBuffer[MACRO_SIZE_MAX];
int _pressedEscapeKeyCount = 0;
uint8_t _isPressedEscapeKey = 0;
uint8_t _isTiredEscapeKey = 0;

static uint8_t _mode;
static uint8_t _step;
static uint8_t _stepAfterLayerSave;	// save 후 실행될 step 저장;
static uint8_t _wait;			// 매크로 완료 후 실행될 작업 구분;
static uint8_t _col, _row;
static uint8_t _buffer[3];
static uint8_t _bufferIndex;
static int _editCount;	// 카운트가 1이상이어야만 eeprom에 write 한다.
static uint8_t _isWorkingForEmpty = 0;	// 매크로 버퍼가 모두 소진된 후 진행할 내용이 있는지 확인;

uint8_t _newKeyMap[ROWS][COLUMNS]; // = {0};

static uint8_t _currentLayer;
static uint8_t _currentLayerAfter;

// 키매핑에 사용되는 키들 정의;
static uint8_t usingKeys[11] = {
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_ESC	//, KEY_S, KEY_X, KEY_L, KEY_R           
};

// define functions
void printPrepareString(void);
static void prepareKeyMapping(void);
void prepareKeyMapper(void);
void saveCurrentLayerAfter(void);

#ifndef	DISABLE_HARDWARE_KEYMAPPING				
void printMapperMessageAfter(void);
#endif
void printSelectModeAfter(void);
void clearMacroAllAfter(void);
void clearMacroAtIndexAfter(void);
void printSelectMode(void);
void readMacro(uint8_t xMacroIndex);
static void addKeymapperDriver(keymapper_driver_t *xDriver);

static keymapper_driver_t *_drivers[4];
static uint8_t _driverCount = 0;
static uint8_t _driverIndex;
static uint8_t _driverIndexOffset;

void initKeymapper(void){
	keyMappingCountMax = setDelay(KEY_MAPPING_COUNT_MAX);

#ifdef DISABLE_HARDWARE_KEYMAPPING
	_driverIndexOffset = 1;
#else
	_driverIndexOffset = 2;
#endif

	addKeymapperDriver(&driverKeymapperLazyFn);
	addKeymapperDriver(&driverKeymapperSmartKey);
	addKeymapperDriver(&driverKeymapperBeyondFn);
}

static void addKeymapperDriver(keymapper_driver_t *xDriver){
	_drivers[_driverCount] = xDriver;
	++_driverCount;
}
void setStep(uint8_t xStep){
	_step = xStep;
}
//------------------------------------------------------///

static void setWillStartKeyMapping(void){
	_isKeyMapping |= BV(0);	//set will start mapping
}

uint8_t isKeyMapping(void){
	return _isKeyMapping & BV(0);	// will start key mapping
}
void setDeepKeyMapping(void){
	_isKeyMapping |= BV(1);	//set doing mapping
}
uint8_t isDeepKeyMapping(void){
	return _isKeyMapping & BV(1);	// did start key mapping
}

static void applyKeyMapping(uint8_t xModi) {
	static uint8_t prevModifier = 0;

	if(isKeyMapping()) return;

	if(xModi == prevModifier) return;

	DEBUG_PRINT(("xModi= %d, _isKeyMapping= %d \n", xModi, _isKeyMapping));

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

void showP2UMenu(void){
	prepareKeyMapping();
}

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
	if(isKeyMapping() && !isDeepKeyMapping() && isReleaseAllPrev()){
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
}


static void countKeyMappingEnabled(void){	
	if(!_isKeyMapping && enabledKeyMappingCount == COUNT_TYPE_COUNTING && ++keyMappingCount > keyMappingCountMax){
		prepareKeyMapping();			
		enabledKeyMappingCount = COUNT_TYPE_KEYMAPPER;
	}

}

void enterFrameForMapper(void){

	applyKeyMapping(getModifierDownBuffer());

	startKeyMapping();

	countKeyMappingEnabled();

	if(isDeepKeyMapping()) {		
		if(_isWorkingForEmpty && !isActiveMacro())
		{
			// DEBUG_PRINT(("_wait : %d \n", _wait));
			if(_wait == WAIT_SAVE){
				saveCurrentLayerAfter();
				// _isWorkingForEmpty = 0;
			}else if(_wait == WAIT_SELECT_MODE){
				printSelectModeAfter();
			}
#ifndef	DISABLE_HARDWARE_KEYMAPPING				
			else if(_wait == WAIT_WELCOME){
				printMapperMessageAfter();
				// _isWorkingForEmpty = 0;
			}
#endif			
			else if(_wait == WAIT_CLEAR_MACRO){
				clearMacroAtIndexAfter();
			}else if(_wait == WAIT_CLEAR_ALL_MACRO){
				clearMacroAllAfter();
				// _isWorkingForEmpty = 0;
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

}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


static void pushCharacter(char *xStr)
{	   
 	// 버퍼에 쌓아두고 모두 출력되기를 기다린다.
	macro_key_t key;

    key = charToKey(xStr[0]); 
    if(key.mode){
    	pushM(KEY_LSHIFT);
    }
	pushM(key.keyindex);	// 같은 코드를 2개 입력한다. 첫번째는 press, 두번재는 release
	pushM(key.keyindex);
    // DEBUG_PRINT(("pushCharacter char %c : %d \n", xStr[0], key.keyindex));
    if(key.mode){
    	pushM(KEY_LSHIFT);
    }
   
}


void printEnter(void)
{	
    pushM(KEY_ENTER);
    pushM(KEY_ENTER);
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
		case STEP_CHOOSE_LAYER:		
		// DEBUG_PRINT(("_currentLayer : %d \n", _currentLayer));	
			printStringFromFlash(str_choose_layer);
			printString(toString(_currentLayer+1));
			printEnter();
		break;
		case STEP_CHOOSE_KEY:
			printStringFromFlash(str_choose_key);
			printEnter();
		break;
		case STEP_INPUT_KEYCODE:
			printStringFromFlash(str_input_keycode);
		break;
		case STEP_SAVE_END_MAPPING:
			printStringFromFlash(str_save_end_mapping);
			printEnter();
			_step = STEP_NOTHING;
		break;
		case STEP_CANCEL_MAPPING:
			printStringFromFlash(str_cancel_mapping);
			printEnter();
			_step = STEP_NOTHING;
		break;
		case STEP_RESET_MAPPING:
			_isWorkingForEmpty = 0;
		break;
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
		case STEP_EXIT_MACRO:
			printStringFromFlash(str_exit_msg);
			printEnter();
			_step = STEP_NOTHING;
		break;
		case STEP_INPUT_MACRO:
			printStringFromFlash(str_input_macro);
			printEnter();
		break;
		case STEP_BOOT_MAPPER:
			printStringFromFlash(str_boot_mapper);
			printEnter();
			_step = STEP_NOTHING;
			setToBootMapper();
		break;
		case STEP_EXIT:
			printStringFromFlash(str_exit_msg);
			printEnter();
			_step = STEP_NOTHING;
		break;
		case STEP_BACK:
			printSelectMode();
		break;
		case STEP_NOTHING:
			// _isWorkingForEmpty = 0;
		break;
	}

	_delay_ms(100);
}
void printSelectModeAfter(void){

	printString(toString(SEL_EXIT));
	printStringFromFlash(str_colon);
	printStringFromFlash(str_select_mode_exit);
	printEnter();
	printString(toString(SEL_BOOT_MAPPER));
	printStringFromFlash(str_colon);
	printStringFromFlash(str_select_mode_bootmapper);
	printEnter();
	
	_step = STEP_SELECT_MODE;
	printPrompt();
}

void printSelectMode(void){	
	printEnter();

	printStringFromFlash(str_select_mode);
	printEnter();

	printString(toString(SEL_MACRO));
	printStringFromFlash(str_colon);
	printStringFromFlash(str_select_mode2);
	printEnter();

#ifndef	DISABLE_HARDWARE_KEYMAPPING
	printString(toString(SEL_MAPPING));
	printStringFromFlash(str_colon);
	printStringFromFlash(str_select_mode1);
	printEnter();
#endif

	uint8_t i;
	for (i = 0; i < _driverCount; ++i)
	{
		printString(toString(i+1+_driverIndexOffset));
		printStringFromFlash(str_colon);
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
	printStringFromFlash(str_mapper_3);
	printEnter();
	printStringFromFlash(str_mapper_4);
	printEnter();
	printString(toString(CMD_BACK));
	printStringFromFlash(str_colon);
	printStringFromFlash(str_back);
	printEnter();
	printStringFromFlash(str_mapper_9);
	printEnter();
	
	_step = STEP_INPUT_COMMAND;
	printPrompt();
}
#endif

#ifndef	DISABLE_HARDWARE_KEYMAPPING				
static void printMapperMessage(void)
{
	printEnter();
	printStringFromFlash(str_mapper_message);
	printEnter();
	printStringFromFlash(str_mapper_1);
	printEnter();
	printStringFromFlash(str_mapper_2);

	// 256 바이트를 넘어 모두 출력하지 못하므로 잘라서 실행;
	_isWorkingForEmpty = 1;
	_wait = WAIT_WELCOME;
	_step = STEP_NOTHING;
}
#endif

void printMacroMessage(void){
	printEnter();
	printStringFromFlash(str_macro_message);
	printEnter();
	printStringFromFlash(str_macro_1);
	printEnter();
	printStringFromFlash(str_macro_2);
	printEnter();
	printString(toString(CMD_EXIT_MACRO));
	printStringFromFlash(str_colon);
	printStringFromFlash(str_exit);
	printEnter();
	printString(toString(CMD_BACK));
	printStringFromFlash(str_colon);
	printStringFromFlash(str_back);
	printEnter();
	printStringFromFlash(str_macro_9);
	printEnter();

	_step = STEP_INPUT_COMMAND;
}


void printPrepareString(void){
	printStringFromFlash(str_prepare_message);
	printEnter();
}

void loadCurrentLayer(void)
{	
	uint16_t gAddress;
	uint8_t k, j;
	for(k = 0; k < ROWS; ++k){
		for (j = 0; j < COLUMNS; ++j)
		{
			gAddress = EEPROM_MAPPING + (k * COLUMNS + j) + (ROWS * COLUMNS * _currentLayer);	// key
			_newKeyMap[k][j] = eeprom_read_byte((uint8_t *)gAddress);
		}
	}
	_editCount = 0;
}

void prepareKeyMapper(void)
{
	memset(_buffer, 0, 3);
	_bufferIndex = 0;
	_currentLayer = 0;
	_mode = 0;

	printSelectMode();
}

void saveCurrentLayerAfter(void)
{
	uint8_t gKeyindex;
	uint16_t gAddress;
	uint8_t k, j;
	for(k = 0; k < ROWS; ++k){
		for (j = 0; j < COLUMNS; ++j)
		{
			gKeyindex = _newKeyMap[k][j];	// value
			gAddress = EEPROM_MAPPING + (k * COLUMNS + j) + (ROWS * COLUMNS * _currentLayer);	// key
			eeprom_write_byte((uint8_t *)gAddress, gKeyindex);
		}
	}
	_editCount = 0;

	if(_stepAfterLayerSave == STEP_SAVE_END_MAPPING)
	{
		_step = _stepAfterLayerSave;		
		stopKeyMapping();
	}else if(_stepAfterLayerSave == STEP_CHOOSE_LAYER)
	{	
		printEnter();
		printStringFromFlash(str_load_layer);
		printEnter();
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

	printStringFromFlash(str_saving);
	printEnter();

	_isWorkingForEmpty = 1;

	_wait = WAIT_SAVE;
	_stepAfterLayerSave = _step;
	_step = STEP_NOTHING;
}

bool isMacroKey(uint8_t xKeyidx){
	if(xKeyidx >= KEY_CST_MAC1 && xKeyidx <= KEY_MAC12){
		return true;
	}else{
		return false;
	}
}

bool isEepromMacroKey(uint8_t xKeyidx){
	if(xKeyidx >= KEY_MAC1 && xKeyidx <= KEY_MAC12){
		return true;
	}else{
		return false;
	}
}

// 매크로 적용됐으면 1, 아니면 0 반환;
uint8_t applyMacro(uint8_t xKeyidx){
	if(isKeyMapping()) return 0;	// 키매핑이 아닐때만 매크로 적용;

	uint8_t gMacroIndex = 255;
	// DEBUG_PRINT(("applyMacro  xKeyidx: %d isMacroKey: %d \n", xKeyidx, isMacroKey(xKeyidx)));
	if(isMacroKey(xKeyidx)){		
		if(!isActiveMacro()){
			if(xKeyidx >= KEY_MAC1){	// eeprom macro
				gMacroIndex = xKeyidx - KEY_MAC1;			
				uint8_t gKeyidx = eeprom_read_byte((uint8_t *)(EEPROM_MACRO+(MACRO_SIZE_MAX * gMacroIndex)));
				if(gKeyidx > 0 && gKeyidx < 255){
					// setActiveMacro(true);
					clearMacroPressedBuffer();
					readMacro(gMacroIndex);
				}
			}else{	// custom macro
				gMacroIndex = xKeyidx - KEY_CST_MAC1;		
				if(hasCustomMacroAt(gMacroIndex)){
					// setActiveMacro(true);
					clearMacroPressedBuffer();
					readCustomMacroAt(gMacroIndex);
				}
			}

			return 1;
		}
	}
	return 0;
}

void readMacro(uint8_t xMacroIndex){
	if(xMacroIndex >= MACRO_NUM) return;
	// DEBUG_PRINT(("readMacro  xMacroIndex: %d \n", xMacroIndex));

	uint16_t gAddress;
	uint8_t gKeyindex;
	uint8_t k;
	for(k = 0; k < MACRO_SIZE_MAX; ++k){
		gAddress = EEPROM_MACRO + (k) + (MACRO_SIZE_MAX * xMacroIndex);	// key
		gKeyindex = eeprom_read_byte((uint8_t *)gAddress);
		if(gKeyindex > 0 && gKeyindex < 255){
			pushM(gKeyindex);
		}
	}
}

void saveMacro(void){
	if(_macroIndex >= MACRO_NUM) return;
		// DEBUG_PRINT(("saveMacro  _macroIndex: %d \n", _macroIndex));

	uint8_t gKeyindex;
	uint16_t gAddress;
	uint8_t k;
	for(k = 0; k < MACRO_SIZE_MAX; ++k){
		gKeyindex = _macroInputBuffer[k];	// value
		gAddress = EEPROM_MACRO + (k) + (MACRO_SIZE_MAX * _macroIndex);	// key
		eeprom_write_byte((uint8_t *)gAddress, gKeyindex);
	}
	// DEBUG_PRINT(("saveMacro  gAddress: %d \n", gAddress));
	_macroIndex = 255;
}

void clearMacroAllAfter(void){
	uint16_t gAddress;
	int k;
	for(k = 0; k < MACRO_SIZE_MAX * MACRO_NUM; ++k){
		gAddress = EEPROM_MACRO + (k);	// key
		eeprom_write_byte((uint8_t *)gAddress, 0);
	}
	// DEBUG_PRINT(("clearMacroAll  gAddress: %d \n", gAddress));

	_step = STEP_INPUT_COMMAND;
	printPrompt();
}

void clearMacroAll(void)
{
	printStringFromFlash(str_clear_all_macro);
	printEnter();

	_isWorkingForEmpty = 1;

	_wait = WAIT_CLEAR_ALL_MACRO;
	_step = STEP_NOTHING;
}

void clearMacroAtIndexAfter(void){
	uint16_t gAddress;
	int k;
	int gLen = MACRO_SIZE_MAX;
	for(k = 0; k < gLen; ++k){
		gAddress = EEPROM_MACRO + (k) + (MACRO_SIZE_MAX * _macroIndex);	// key
		eeprom_write_byte((uint8_t *)gAddress, 0);
	}
	_step = STEP_INPUT_COMMAND;
	printPrompt();
}

void clearMacroAtIndex(void){	
	printStringFromFlash(str_macro);
	printStringFromFlash(str_space);
	printString(toString(_macroIndex+1));
	printStringFromFlash(str_space);
	printStringFromFlash(str_clear_all_macro);
	printEnter();

	_isWorkingForEmpty = 1;

	_wait = WAIT_CLEAR_MACRO;
	_step = STEP_NOTHING;
}

void resetCurrentLayer(void)
{
	printStringFromFlash(str_reset_mapping);
	printEnter();
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

bool isMacroInput(void){
	if(_step == STEP_INPUT_MACRO){
		return true;
	}else{
		return false;
	}
}

static uint8_t _macroDownCount = 0;
static bool _isQuickMacro = false;
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
	_step = STEP_INPUT_MACRO;
	setDeepKeyMapping();
	_isQuickMacro = true;
	blinkOnce(500);
	_delay_ms(100);
	blinkOnce(500);
}
void stopQuickMacro(void){
	saveMacro();
	_macroIndex = 255;
	_step = STEP_NOTHING;
	_isQuickMacro = false;
	stopKeyMapping();
	blinkOnce(1000);
}

static void stopMacroInput(void){
	if(_isQuickMacro){
		stopQuickMacro();
	}else{
		saveMacro();
		_step = STEP_INPUT_COMMAND;
		printEnter();
		printPrompt();
	}
}

void putMacro(uint8_t xKeyidx, uint8_t xIsDown){ 
    int gIdx;

	if(xKeyidx >= KEY_MAX) return;		// 매크로 입력시 키값으로 변환할 수 없는 특수 키들은 중단;

	if(_isTiredEscapeKey){
		_isPressedEscapeKey = 0;
		_isTiredEscapeKey = 0;

		pushM(xKeyidx);
		// 또는 ESC를 1초이상 누르면 종료;
		_macroInputBuffer[_macroBufferIndex-1] = 0;	// 마지막 esc 눌림 제거;

		stopMacroInput();

		return;
	}

	if(xIsDown){
		if(_macroDownCount >= MACRO_SIZE_MAX_HALF){	// 매크로 크기의 절반이 넘은 키 다운은 제외 시킨다. 그래야 나머지 공간에 up 데이터를 넣을 수 있으므로.
			return;
		}
		++_macroDownCount;
	    // DEBUG_PRINT(("down _macroDownCount : %d xKeyidx : %d \n", _macroDownCount, xKeyidx));
	    DBG1(0x07, (uchar *)&xKeyidx, 1);  
	   
	    append(_macroPressedBuffer, xKeyidx);
	    
	}else{
		// gLen = strlen((char *)_macroPressedBuffer);
	    gIdx = findIndex(_macroPressedBuffer, xKeyidx);
	    // 릴리즈시에는 프레스 버퍼에 있는 녀석만 처리; 버퍼에 없는 녀석은 16키 이후의 키이므로 제외;
	    if(gIdx == -1){
	    	return;
	    }
	    delete(_macroPressedBuffer, gIdx);
	    // DEBUG_PRINT(("up idx : %d, buffer len : %d xKeyidx : %d \n", gIdx, strlen((char *)_macroPressedBuffer), xKeyidx));
	    DBG1(0x08, (uchar *)&xKeyidx, 1);  
	}

	_macroInputBuffer[_macroBufferIndex] = xKeyidx;
	++_macroBufferIndex;

	DBG1(0x09, (uchar *)&_macroInputBuffer, strlen((char *)_macroInputBuffer));  

	pushM(xKeyidx);

	// DEBUG_PRINT(("                                              _macroBufferIndex : %d \n", _macroBufferIndex));

	// MACRO_SIZE_MAX개를 채웠다면 종료;
	if(_macroBufferIndex >= MACRO_SIZE_MAX){
		// _macroIndex 위치에 저장;
		// DEBUG_PRINT((".......................... macro input end \n"));
	
		stopMacroInput();

		return;
	}

	if(xIsDown && xKeyidx == KEY_ESC){
		//esc 눌렸음;
		_isPressedEscapeKey = 1;
		_pressedEscapeKeyCount = 0;
	}else{
		_isPressedEscapeKey = 0;
		_isTiredEscapeKey = 0;
	}
}

void putKeyindex(uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, uint8_t xIsDown)
{	
	uint8_t cmd,gLayer;
	int gKeyCode;
	int gKeyIndex; 
    int gIdx;

	// 매크로 실행중에는 입력을 받지 않는다.
	if(_isWorkingForEmpty) return;

    xKeyidx = getDualActionKeyWhenCompound(xKeyidx);

	// 매핑 중에는 키 업만 실행 시킨다.
	if(!isMacroInput() && xIsDown) return;	// 매크로 일 경우에만 다운 키 실행;

	if(isMacroInput()){
		putMacro(xKeyidx, xIsDown);	

		return;	
	}

	gKeyIndex = findIndex(usingKeys, xKeyidx);

	// 입력되는 키코드 값은 usb/ps2 모두 동일하다.
	// DEBUG_PRINT(("keymapping xKeyidx: %02x, gKeyIndex: %d, col: %d, row: %d \n", xKeyidx, gKeyIndex, xCol, xRow));
	if(_step == STEP_INPUT_COMMAND || _step == STEP_SELECT_MODE){
		if(gKeyIndex > -1 && gKeyIndex < 10){	// 0~9
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
					_mode = SEL_EXIT;
					_step = STEP_EXIT;
					stopKeyMapping();
				}else if(cmd == SEL_BOOT_MAPPER){
					_mode = SEL_BOOT_MAPPER;
					_step = STEP_BOOT_MAPPER;
					stopKeyMapping();				
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
				}else if(_mode == SEL_MACRO){
					if(cmd == CMD_SELECT_INDEX){
						_step = STEP_SELECT_INDEX;
					}else if(cmd == CMD_CLEAR_MACRO){
						_step = STEP_CLEAR_SELECT_INDEX;
					}else if(cmd == CMD_EXIT_MACRO){
						_step = STEP_EXIT_MACRO;
						stopKeyMapping();
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
		return;

	}else if(_step == STEP_CHOOSE_KEY){
		// do not anything;
	}else if(gKeyIndex > -1 && gKeyIndex < 10){	// 0~9

		_buffer[_bufferIndex] = gKeyIndex;
		++_bufferIndex;

		// sprintf(gStr, "%d", gKeyIndex);
		printString(toString(gKeyIndex));		

	}else{
		// DEBUG_PRINT(("bad command \n"));
		return;
	}
	
	// DEBUG_PRINT(("step: %d, buffer [0]: %d, [1]: %d, [2]: %d, index:%d \n", _step, _buffer[0], _buffer[1], _buffer[2], _bufferIndex));
	
	// key mapper
	if((gKeyIndex > -1  && gKeyIndex < 10) || _step == STEP_CHOOSE_KEY)
	{		
		switch(_step){
			case STEP_SELECT_INDEX:
				if(_bufferIndex == 2)
				{
					gKeyCode = (_buffer[0] * 10);
					if(gKeyCode > -1){
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
					return;
				}
			break;	
			case STEP_CLEAR_SELECT_INDEX:
				if(_bufferIndex == 2)
				{
					gKeyCode = (_buffer[0] * 10);
					if(gKeyCode > -1){
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
						printStringFromFlash(str_invalid_number);
						printEnter();
					}

					memset(_buffer, 0, 3);
					_bufferIndex = 0;
				}else{
					_delay_ms(100);
					return;
				}
			break;	
			case STEP_CHOOSE_LAYER:
			
				gLayer = _buffer[0] - 1;
				// DEBUG_PRINT(("________________________________________ gLayer : %d \n", gLayer));
				if(_currentLayer < 3 && _currentLayer != gLayer && (gLayer >= 0 && gLayer < 3)){
					if(_editCount > 0){
						printEnter();
						printStringFromFlash(str_save_layer);
						printEnter();
						saveCurrentLayer();
						_currentLayerAfter = gLayer;
					}else{
						printEnter();
						printStringFromFlash(str_load_layer);
						printEnter();
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
					if(gKeyCode > -1){
						gKeyCode = gKeyCode + (_buffer[1] * 10);
					}
					if(gKeyCode > -1){
						gKeyCode = gKeyCode + _buffer[2];
					}

					// DEBUG_PRINT(("_currentLayer: %d, gKeyCode : %d, _bufferIndex: %d \n", _currentLayer, gKeyCode, _bufferIndex));

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
					return;
				}

			break;
		}
		printPrompt();
	}

}

#endif
