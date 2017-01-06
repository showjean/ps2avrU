
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "keyindex.h"
#include "hardwareinfo.h"
#include "eeprominfo.h"
#include "keymap.h"
#include "quickmacro.h"
#include "quickswap.h"

static uint8_t escapeMacroKeyindex(uint8_t xKeyidx){
	if(isMacroKey(xKeyidx)){
		return 0;
	}
	return xKeyidx;
}

// flashrom의 기본 키코드 반환(부트 매퍼);
uint8_t getDefaultKeyindex(uint8_t xLayer, uint8_t xRow, uint8_t xCol)
{
	return pgm_read_byte(KEYMAP_ADDRESS+(ROWS * COLUMNS * xLayer)+(xRow * COLUMNS + xCol));
}

uint8_t getExchangedKeyindex(uint8_t xKeyindex){

//	return getQuickSwapKeyindex(getSmartKeyIndex(xKeyindex));
	return getQuickSwapKeyindex(xKeyindex);
}

// 키들을 순서대로 나열한 인덱스를 반환. <키코드가 아님!>
uint8_t getCurrentKeyindex(uint8_t xLayer, uint8_t xRow, uint8_t xCol)
{
	uint8_t gKeyIndex = 0;

		if(isMacroInput()){			
			// 매크로 입력 중에는 매핑된 키코드를 사용;
            gKeyIndex = getDefaultKeyindex(xLayer, xRow, xCol);

			// 매크로 입력 중에는 다시 매크로 키가 포함되지 않도록;
			gKeyIndex = escapeMacroKeyindex(gKeyIndex);

        // return gKeyIndex;
        goto RETURN_INDEX;
	}

	if(gKeyIndex < 1 || gKeyIndex > 254){
		gKeyIndex = getDefaultKeyindex(xLayer, xRow, xCol);
	}

RETURN_INDEX:
	gKeyIndex = getExchangedKeyindex(gKeyIndex);

	return gKeyIndex;
}
