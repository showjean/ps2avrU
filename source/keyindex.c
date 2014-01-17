
#include "keyindex.h"
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "hardwareinfo.h"
#include "eeprominfo.h"
#include "keymap.h"
#include "keymapper.h"
#include "quickswap.h"


uint8_t* keymapAddress = (uint8_t*)KEYMAP_ADDRESS;

// eeprom에 매핑된 키코드 반환(하드웨어 키매핑)
static uint8_t getMappingKeyindex(uint8_t xLayer, uint8_t xRow, uint8_t xCol)
{

#ifndef	DISABLE_HARDWARE_KEYMAPPING
	uint8_t gKeyIndex;
	int gIdx;
	gIdx = EEPROM_MAPPING + (xRow * COLUMNS + xCol) + (COLUMNS * ROWS * xLayer);
	gKeyIndex = eeprom_read_byte((uint8_t *)gIdx);

	return gKeyIndex;
#else
	return 0;
#endif

}

static uint8_t escapeMacroKeyindex(uint8_t xKeyidx){
	if(isMacroKey(xKeyidx)){
		return 0;
	}
	return xKeyidx;
}

uint8_t getKeyIndex(uint8_t xLayer, uint8_t xRow, uint8_t xCol){
	return  pgm_read_byte(keymapAddress+(ROWS * COLUMNS * xLayer)+(xRow * COLUMNS + xCol));
}

// flashrom의 기본 키코드 반환(부트 매퍼);
uint8_t getDefaultKeyindex(uint8_t xLayer, uint8_t xRow, uint8_t xCol)
{
	return getKeyIndex(xLayer, xRow, xCol);
}

// 키들을 순서대로 나열한 인덱스를 반환. <키코드가 아님!>
uint8_t getCurrentKeyindex(uint8_t xLayer, uint8_t xRow, uint8_t xCol)
{
	uint8_t gKeyIndex;
	if(isDeepKeyMapping() ) {	// 키매핑 중에는 달리 처리;
		
		if(isMacroInput()){			
			// 매크로 입력 중에는 매핑된 키코드를 사용;
			// 매크로 입력 중에는 다시 매크로 키가 포함되지 않도록;
			gKeyIndex = getMappingKeyindex(xLayer, xRow, xCol);	
			gKeyIndex = escapeMacroKeyindex(gKeyIndex);
		}else{
			// 키 매핑 중에는 숫자키만을 이용하므로, 기본 키맵을 이용하도록 한다.
			gKeyIndex = getDefaultKeyindex(xLayer, xRow, xCol);

			return gKeyIndex;
		}
	}else{
		gKeyIndex = getMappingKeyindex(xLayer, xRow, xCol);	
	}

	if(gKeyIndex < 1 || gKeyIndex > 254){		
		gKeyIndex = getDefaultKeyindex(xLayer, xRow, xCol);
	}

	gKeyIndex = getQuickSwapKeyindex(gKeyIndex);

	return gKeyIndex;
}
