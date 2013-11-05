#ifndef KEYMAPPER_C
#define KEYMAPPER_C

#include "timer.h"
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
#include "hardwareinfo.h"
#include "keymap.h"
#include "keymatrix.h"
#include "ps2avru_util.h"
#include "macrobuffer.h"
#include "ps2main.h"
#include "usbmain.h"

const char str_select_mode[] PROGMEM =  "select mode";
const char str_select_mode1[] PROGMEM =  "1:Key Mapping";
const char str_select_mode2[] PROGMEM =  "2:Macro";
const char str_select_mode3[] PROGMEM =  "3:Exit";
const char str_exit[] PROGMEM =  "good bye~";

const char str_macro_message[] PROGMEM = "Macro";
const char str_macro_1[] PROGMEM = "1:Select Macro Index";
const char str_macro_2[] PROGMEM = "2:Exit";
const char str_macro_9[] PROGMEM = "9:Clear All";

const char str_back_6[] PROGMEM = "6:Back";

const char str_mapper_message[] PROGMEM = "Key Mapper";
const char str_mapper_1[] PROGMEM = "1:Change Layer";
const char str_mapper_2[] PROGMEM = "2:Select Key - input keycode";
const char str_mapper_3[] PROGMEM = "3:Save and Exit";
const char str_mapper_4[] PROGMEM = "4:Exit without Saving";
const char str_mapper_9[] PROGMEM = "9:Reset to Default (Current Layer)";

const char str_prepare_message[] PROGMEM =  "ps2avrU";
const char str_choose_layer[] PROGMEM =  "choose layer (1: normal, 2: FN, 3: FN2) current= ";
const char str_choose_key[] PROGMEM = "select key (any key you want) ";
const char str_input_keycode[] PROGMEM = "input keycode (must 3 numbers) : ";
const char str_save_end_mapping[] PROGMEM = "Save & Exit, thank you.";
const char str_cancel_mapping[] PROGMEM = "Exit without Saving, see you later.";
const char str_reset_mapping[] PROGMEM = "Reset to default current layer";
const char str_input_command[] PROGMEM = "Input Command Number: ";
const char str_nothing[] PROGMEM = "nothing";
const char str_save_layer[] PROGMEM = "Save current layer";
const char str_load_layer[] PROGMEM = "Load current layer";
const char str_saving[] PROGMEM = "saving...";
const char str_col_row[] PROGMEM = "col, row : ";
const char str_invalid_keycode[] PROGMEM = "invalid keycode, input again.";

const char str_select_number[] PROGMEM = "input Macro Index (01~12, must 2 numbers) : ";
const char str_exit_macro[] PROGMEM = "Exit macro, see you later.";
const char str_input_macro[] PROGMEM = "input key (max 16 keys, stop : press ESC during 1 sec)";
const char str_invalid_macro_number[] PROGMEM = "invalid Index, input again.";
const char str_clear_all_macro[] PROGMEM = "clear...";


static uint8_t _isKeyMapping = 0;	// 0b00000001 : will start mapping, 0b00000011 : did start mapping
static uint8_t enabledKeyMappingCount = 0;
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
static uint8_t _stepAfterSave;	// save 후 실행될 step 저장;
static uint8_t _wait;			// 매크로 완료 후 실행될 작업 구분;
static uint8_t _col, _row;
static uint8_t _buffer[3];
static uint8_t _bufferIndex;
static int _editCount;	// 카운트가 1이상이어야만 eeprom에 write 한다.
static uint8_t _isWorkingForEmpty = 0;	// 매크로 버퍼가 모두 소진된 후 진행할 내용이 있는지 확인;

uint8_t _newKeyMap[17][8]; // = {0};

static uint8_t _currentLayer;
static uint8_t _currentLayerAfter;

// 키매핑에 사용되는 키들 정의;
static uint8_t usingKeys[11] = {
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_ESC	//, KEY_S, KEY_X, KEY_L, KEY_R           
};

// define functions
void printPrepareString(void);
void prepareKeyMapper(void);
void saveCurrentLayerAfter(void);
void printMapperMessageAfter(void);
void clearMacroAfter(void);
void printSelectMode(void);
void readMacro(uint8_t xMacroIndex);

//------------------------------------------------------///

void setWillStartKeyMapping(void){
	_isKeyMapping |= BV(0);	//set will start mapping
}

uint8_t isKeyMapping(void){
	return _isKeyMapping & BV(0);	// will start key mapping
}
uint8_t isDeepKeyMapping(void){
	return _isKeyMapping & BV(1);	// did start key mapping
}

void applyKeyMapping(uint8_t xModi) {
	if(isKeyMapping()) return;

		DEBUG_PRINT(("xModi= %d, _isKeyMapping= %d \n", xModi, _isKeyMapping));
	// 약 5초간 입력이 지속되면 키매핑 모드로
	if(xModi == 0x07){
		keyMappingCount = 0;
		enabledKeyMappingCount = 1;

		if(INTERFACE == INTERFACE_PS2 || INTERFACE == INTERFACE_PS2_USER){		
			keyMappingCountMax = KEY_MAPPING_COUNT_MAX >> 1;	// ps2의 경우 USB보다 대기 시간이 길어서 반으로 줄여줌;
		}
	}else{		
		_isKeyMapping = 0;
		enabledKeyMappingCount = 0;
	}
}

/**
 진입키(Left - ctrl+alt+shift)가 입력되면 매핑 시작을 준비한다.
*/
void prepareKeyMapping(void){
	setWillStartKeyMapping();	//set will start mapping

	// 각 인터페이스 준비;
	if(INTERFACE == INTERFACE_USB || INTERFACE == INTERFACE_USB_USER){
	    prepareKeyMappingUsb();
	}else{
		prepareKeyMappingPs2();
		keyMappingCountMax = KEY_MAPPING_COUNT_MAX >> 1;
	}
	// DEBUG_PRINT(("prepareKeyMapping : _isKeyMapping= %d \n", _isKeyMapping));

	printPrepareString();

}
static void startKeyMappingDeep(void)
{
	_isKeyMapping |= BV(1);	//set doing mapping
	// DEBUG_PRINT(("startKeyMapping : _isKeyMapping= %d \n", _isKeyMapping));
	prepareKeyMapper();
}

uint8_t isReadyKeyMappingOnBoot(void){
	return _keyMappingOnBoot;
}

// 부팅시 키매핑 시작
void readyKeyMappingOnBoot(void)
{
	_keyMappingOnBoot = 1;
	// setWillStartKeyMapping();
}

void startKeyMappingOnBoot(void)
{
	if(_keyMappingOnBoot == 1){
		prepareKeyMapping();
		_keyMappingOnBoot = 0;
	}
}

/**
매핑 준비가 되었을 때 모든키의 입력이 해제 되면 본격적으로 매핑을 시작한다.
*/
void startKeyMapping(void){
	if(isKeyMapping() && !isDeepKeyMapping() && isAllKeyRelease()){
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


void countKeyMappingEnabled(void){	
	if(!_isKeyMapping && enabledKeyMappingCount && ++keyMappingCount > keyMappingCountMax){
		enabledKeyMappingCount = 0;
		prepareKeyMapping();
	}

}

void enterFrameForMapper(void){
	countKeyMappingEnabled();

	if(isDeepKeyMapping()) {		
		if(_isWorkingForEmpty && isMacroProcessEnd())
		{
			DEBUG_PRINT(("_wait : %d \n", _wait));
			if(_wait == WAIT_SAVE){
				saveCurrentLayerAfter();
				// _isWorkingForEmpty = 0;
			}else if(_wait == WAIT_WELCOME){
				printMapperMessageAfter();
				// _isWorkingForEmpty = 0;
			}else if(_wait == WAIT_CLEAR_MACRO){
				clearMacroAfter();
				// _isWorkingForEmpty = 0;
			}else{
				_isWorkingForEmpty = 0;
			}
			_wait = WAIT_NOTHING;
		}
		
		if(_isPressedEscapeKey && ++_pressedEscapeKeyCount > 1000){
			_isTiredEscapeKey = 1;
			putKeyCode(KEY_ESC, 255, 255, 0);	// esc가 up 된것으로 알림;
		}
	}

}

static uint8_t getDefaultKeyCode(uint8_t xLayer, uint8_t xRow, uint8_t xCol)
{
	return pgm_read_byte(&keymap_code[xLayer][xRow][xCol]);
}

static uint8_t getMappingKeyCode(uint8_t xLayer, uint8_t xRow, uint8_t xCol)
{
	uint8_t gKeyIndex;
	int gIdx;
	gIdx = EEPROM_MAPPING + (xRow * 8 + xCol) + (136 * xLayer);
	gKeyIndex = eeprom_read_byte((uint8_t *)gIdx);
	return gKeyIndex;
}

uint8_t isMacroKey(uint8_t xKeyCode){
	if(xKeyCode >= KEY_MAC1 && xKeyCode <= KEY_MAC12){
		return 1;
	}else{
		return 0;
	}
}

uint8_t escapeMacroKeycode(uint8_t xKeyCode){
	if(isMacroKey(xKeyCode)){
		return 0;
	}
	return xKeyCode;
}

uint8_t getCurrentKeycode(uint8_t xLayer, uint8_t xRow, uint8_t xCol)
{
	uint8_t gKeyIndex;
	if(_isKeyMapping ) {//&& _step != STEP_INPUT_MACRO){	// 키코드 입력 모드이거나 매크로 입력중이 아닐때 기본값을 이용한다.
		// 기본 숫자키에 펌웨어로 매크로 키를 지정하면 제대로 작동되지 않는다.
		// 또한 전혀 다른 레이아웃의 키보드에 와이어링한 기판이라면 매트릭스가 다를테니 기본값도 무의미하다. 이 경우 애초에 매핑도 하지 못하니 일단 제외.
		gKeyIndex = getDefaultKeyCode(xLayer, xRow, xCol);
		return gKeyIndex;
	}

	gKeyIndex = getMappingKeyCode(xLayer, xRow, xCol);	
	
	if(_isKeyMapping && _step == STEP_INPUT_MACRO){
		// 매크로 입력 중에는 다시 매크로 키가 포함되지 않도록;
		gKeyIndex = escapeMacroKeycode(gKeyIndex);
	}

	if(gKeyIndex == 0 || gKeyIndex >= 255){		
		gKeyIndex = getDefaultKeyCode(xLayer, xRow, xCol);
	}

	return gKeyIndex;
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


static void pushCharacter(char *xStr)
{	   
 	// 버퍼에 쌓아두고 모두 출력되기를 기다린다.
	Key key;

    key = charToKey(xStr[0]); 
    if(key.mode){
    	pushM(KEY_LSHIFT);
    }
	pushM(key.keycode);	// 같은 코드를 2개 입력한다. 첫번째는 press, 두번재는 release
	pushM(key.keycode);
    // DEBUG_PRINT(("pushCharacter char %c : %d \n", xStr[0], key.keycode));
    if(key.mode){
    	pushM(KEY_LSHIFT);
    }
   
}


static void printEnter(void)
{	
    pushM(KEY_ENTER);
    pushM(KEY_ENTER);
}

static void printString(const char *xString)
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
static void printStringFromFlash(const char *str) 
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
			// sprintf(gStr, "%d", _currentLayer+1);
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
		case STEP_EXIT_MACRO:
			printStringFromFlash(str_exit_macro);
			printEnter();
			_step = STEP_NOTHING;
		break;
		case STEP_INPUT_MACRO:
			printStringFromFlash(str_input_macro);
			printEnter();
		break;
		case STEP_EXIT:
			printStringFromFlash(str_exit);
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

void printSelectMode(void){
	printEnter();
	printStringFromFlash(str_select_mode);
	printEnter();
	printStringFromFlash(str_select_mode1);
	printEnter();
	printStringFromFlash(str_select_mode2);
	printEnter();
	printStringFromFlash(str_select_mode3);
	printEnter();
	
	_step = STEP_SELECT_MODE;
	printPrompt();
}

void printMapperMessageAfter(void)
{
	printEnter();
	printStringFromFlash(str_mapper_3);
	printEnter();
	printStringFromFlash(str_mapper_4);
	printEnter();
	printStringFromFlash(str_back_6);
	printEnter();
	printStringFromFlash(str_mapper_9);
	printEnter();
	
	_step = STEP_INPUT_COMMAND;
	printPrompt();

}
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
void printMacroMessage(void){
	printEnter();
	printStringFromFlash(str_macro_message);
	printEnter();
	printStringFromFlash(str_macro_1);
	printEnter();
	printStringFromFlash(str_macro_2);
	printEnter();
	printStringFromFlash(str_back_6);
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
	for(k = 0; k < 17; ++k){
		for (j = 0; j < 8; ++j)
		{
			gAddress = EEPROM_MAPPING + (k * 8 + j) + (136 * _currentLayer);	// key
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

	loadCurrentLayer();

	printSelectMode();
}

void saveCurrentLayerAfter(void)
{
	uint8_t gKeyCode;
	uint16_t gAddress;
	uint8_t k, j;
	for(k = 0; k < 17; ++k){
		for (j = 0; j < 8; ++j)
		{
			gKeyCode = _newKeyMap[k][j];	// value
			gAddress = EEPROM_MAPPING + (k * 8 + j) + (136 * _currentLayer);	// key
			eeprom_write_byte((uint8_t *)gAddress, gKeyCode);
		}
	}
	_editCount = 0;

	if(_stepAfterSave == STEP_SAVE_END_MAPPING)
	{
		_step = _stepAfterSave;		
		stopKeyMapping();
	}else if(_stepAfterSave == STEP_CHOOSE_LAYER)
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
	_stepAfterSave = _step;
	_step = STEP_NOTHING;
}

// 매크로 적용됐으면 1, 아니면 0 반환;
uint8_t applyMacro(uint8_t xKeyidx){
	if(isKeyMapping()) return 0;	// 키매핑이 아닐때만 매크로 적용;

	uint8_t gMacroIndex = 255;
	// DEBUG_PRINT(("applyMacro  xKeyidx: %d isMacroKey: %d \n", xKeyidx, isMacroKey(xKeyidx)));
	if(isMacroKey(xKeyidx) && isEmptyM()){
		gMacroIndex = xKeyidx - KEY_MAC1;
		clearMacroPressedBuffer();
		readMacro(gMacroIndex);

		return 1;
	}
	return 0;
}

void readMacro(uint8_t xMacroIndex){
	if(xMacroIndex > 11) return;
	// DEBUG_PRINT(("readMacro  xMacroIndex: %d \n", xMacroIndex));

	uint16_t gAddress;
	uint8_t gKeyCode;
	uint8_t buffers[MACRO_SIZE_MAX];
	uint8_t k;
	for(k = 0; k < MACRO_SIZE_MAX; ++k){
		gAddress = EEPROM_MACRO + (k) + (MACRO_SIZE_MAX * xMacroIndex);	// key
		gKeyCode = eeprom_read_byte((uint8_t *)gAddress);
		buffers[k] = gKeyCode;
		if(gKeyCode > 0 && gKeyCode < 255){
			pushM(gKeyCode);
		}
	}
	DEBUG_PRINT(("readMacro  buffers[0]: %d, buffers[1]: %d, buffers[30]: %d, buffers[31]: %d \n", buffers[0], buffers[1], buffers[30], buffers[31]));
}

void saveMacro(void){
	if(_macroIndex > 11) return;
		// DEBUG_PRINT(("saveMacro  _macroIndex: %d \n", _macroIndex));

	uint8_t gKeyCode;
	uint16_t gAddress;
	uint8_t buffers[MACRO_SIZE_MAX];
	uint8_t k;
	for(k = 0; k < MACRO_SIZE_MAX; ++k){
		gKeyCode = _macroInputBuffer[k];	// value
		gAddress = EEPROM_MACRO + (k) + (MACRO_SIZE_MAX * _macroIndex);	// key
		buffers[k] = gKeyCode;
		eeprom_write_byte((uint8_t *)gAddress, gKeyCode);
	}
	// DEBUG_PRINT(("saveMacro  gAddress: %d \n", gAddress));
	// DEBUG_PRINT(("saveMacro  buffers[0]: %d, buffers[1]: %d, buffers[30]: %d, buffers[31]: %d \n", buffers[0], buffers[1], buffers[30], buffers[31]));
	_macroIndex = 255;
}

void clearMacroAfter(void){
	uint16_t gAddress;
	int k;
	for(k = 0; k < MACRO_SIZE_MAX * 12; ++k){
		gAddress = EEPROM_MACRO + (k);	// key
		eeprom_write_byte((uint8_t *)gAddress, 0);
	}
	// DEBUG_PRINT(("clearMacro  gAddress: %d \n", gAddress));

	_step = STEP_INPUT_COMMAND;
	printPrompt();
}

void clearMacro(void)
{
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
	for(k = 0; k < 17; ++k){
		for (j = 0; j < 8; ++j)
		{
			_newKeyMap[k][j] = 0;
		}
	}
	_editCount = 1;
		
}

static uint8_t _macroDownCount = 0;
void resetMacroInput(void){	
	memset(_macroInputBuffer, 0, MACRO_SIZE_MAX);
	_macroBufferIndex = 0;
	memset(_macroPressedBuffer, 0, MACRO_SIZE_MAX);
	_macroDownCount = 0;
}
void putKeyCode(uint8_t xKeyCode, uint8_t xCol, uint8_t xRow, uint8_t xIsDown)
{
	// 매크로 실행중에는 입력을 받지 않는다.
	if(_isWorkingForEmpty) return;
	// DEBUG_PRINT(("putKeyCode xKeyCode: %02x, xIsDown: %d, col: %d, row: %d \n", xKeyCode, xIsDown, xCol, xRow));

	// 매핑 중에는 키 업만 실행 시킨다.
	if(_step != STEP_INPUT_MACRO && xIsDown) return;	// 매크로 일 경우에만 다운 키 실행;

	uint8_t cmd,gLayer;
	int gKeyCode;
	int gKeyIndex; 
	char gStr[3];
    int gIdx;
    int gLen;

	gKeyIndex = findIndex(usingKeys, 11, xKeyCode);

	if(_step == STEP_INPUT_MACRO){
		if(_isTiredEscapeKey){
			_isPressedEscapeKey = 0;
			_isTiredEscapeKey = 0;

			pushM(xKeyCode);
			// 또는 ESC를 1초이상 누르면 종료;
			_macroInputBuffer[_macroBufferIndex-1] = 0;	// 마지막 esc 눌림 제거;
			saveMacro();
			_step = STEP_INPUT_COMMAND;
			printEnter();
			printPrompt();

			return;
		}

		if(xIsDown){
			if(_macroDownCount >= MACRO_SIZE_MAX_HALF){	// 매크로 크기의 절반이 넘은 키 다운은 제외 시킨다. 그래야 나머지 공간에 up 데이터를 넣을 수 있으므로.
				return;
			}
			++_macroDownCount;
		    DEBUG_PRINT(("down _macroDownCount : %d \n", _macroDownCount));
		   
		    append(_macroPressedBuffer, xKeyCode);
		    
		}else{
			gLen = strlen((char *)_macroPressedBuffer);
		    gIdx = findIndex(_macroPressedBuffer, gLen, xKeyCode);
		    // 릴리즈시에는 프레스 버퍼에 있는 녀석만 처리; 버퍼에 없는 녀석은 16키 이후의 키이므로 제외;
		    if(gIdx == -1){
		    	return;
		    }
		    delete(_macroPressedBuffer, gIdx);
		    DEBUG_PRINT(("up idx : %d, buffer len : %d \n", gIdx, strlen((char *)_macroPressedBuffer)));
		}

		_macroInputBuffer[_macroBufferIndex] = xKeyCode;
		++_macroBufferIndex;

		pushM(xKeyCode);

		DEBUG_PRINT(("                                              _macroBufferIndex : %d \n", _macroBufferIndex));

		// MACRO_SIZE_MAX개를 채웠다면 종료;
		if(_macroBufferIndex >= MACRO_SIZE_MAX){
			_step = STEP_INPUT_COMMAND;
			// _macroIndex 위치에 저장;
			DEBUG_PRINT((".......................... macro input end \n"));
			saveMacro();
			printEnter();
			printPrompt();

			return;
		}

		if(xIsDown && usingKeys[gKeyIndex] == KEY_ESC){
			//esc 눌렸음;
			_isPressedEscapeKey = 1;
			_pressedEscapeKeyCount = 0;
		}else{
			_isPressedEscapeKey = 0;
			_isTiredEscapeKey = 0;
		}

		return;
	}


	// 입력되는 키코드 값은 usb/ps2 모두 동일하다.
	// DEBUG_PRINT(("keymapping xKeyCode: %02x, gKeyIndex: %d, col: %d, row: %d \n", xKeyCode, gKeyIndex, xCol, xRow));
	if(_step == STEP_INPUT_COMMAND || _step == STEP_SELECT_MODE){
		if(gKeyIndex > -1 && gKeyIndex < 10){	// 0~9
			_buffer[_bufferIndex] = gKeyIndex;
		
			sprintf(gStr, "%d", gKeyIndex);
			printString(gStr);
			printEnter();

			// 커맨드에 따라서 스텝 지정;
			cmd = _buffer[0];

			if(_step == STEP_SELECT_MODE){
				if(cmd == SEL_MAPPING){
					_mode = SEL_MAPPING;
					printMapperMessage();
				}else if(cmd == SEL_MACRO){
					_mode = SEL_MACRO;
					printMacroMessage();
				}else if(cmd == SEL_EXIT){
					_mode = SEL_EXIT;
					_step = STEP_EXIT;
					stopKeyMapping();
				}
			}else{
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
						// 13 + (row * 8 + col) + (136 * layer)
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
					}else if(cmd == CMD_EXIT_MACRO){
						_step = STEP_EXIT_MACRO;
						stopKeyMapping();
					}else if(cmd == CMD_CLEAR_ALL_MACRO){
						_step = STEP_CLEAR_ALL_MACRO;
						clearMacro();
					}
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

		sprintf(gStr, "%d", gKeyIndex);
		printString(gStr);		

	}else{
		// DEBUG_PRINT(("bad command \n"));
		return;
	}
	
	// DEBUG_PRINT(("step: %d, buffer [0]: %d, [1]: %d, [2]: %d, index:%d \n", _step, _buffer[0], _buffer[1], _buffer[2], _bufferIndex));
	
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

					if(gKeyCode > 0 && gKeyCode <= 12){
						_macroIndex = gKeyCode - 1;
						_step = STEP_INPUT_MACRO;
						resetMacroInput();
					}else{
						printEnter();
						printStringFromFlash(str_invalid_macro_number);
					}

					memset(_buffer, 0, 3);
					_bufferIndex = 0;
					printEnter();
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
				gKeyCode = getDefaultKeyCode(_currentLayer, _row, _col);
				printString(toString(gKeyCode));
				printString(" mapping=");
				gKeyCode = _newKeyMap[_row][_col]; //getMappingKeyCode(_currentLayer, _row, _col);
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
						printStringFromFlash(str_invalid_keycode);
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
