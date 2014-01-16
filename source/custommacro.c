
#include <avr/pgmspace.h>
#include "custommacro.h"
#include "hardwareinfo.h"
#include "macrobuffer.h"
#include "print.h"

uint8_t* macroAddress = (uint8_t*)CUSTOM_MACRO_ADDRESS;

bool hasCustomMacroAt(uint8_t xMacroIndex){
	uint8_t gKeyidx = pgm_read_byte(macroAddress+(CUSTOM_MACRO_SIZE_MAX * xMacroIndex));
	if(gKeyidx > 0 && gKeyidx < 255){
		return true;
	}
	return false;
}

// 매크로가 실행되면 timer2를 작동시켜서 정해진 delay마다 매크로 버퍼에 keyindex를 넣어 준다.
// 해당 매크로의 실행이 완료되기 전까지 다른 키입력은 막는다.
// 단, 현재는 딜레이를 적용하지 않았으므로 모든 keyindex를 매크로 버퍼에 넣는다.
void readCustomMacroAt(uint8_t xMacroIndex){
	// DEBUG_PRINT(("readCustomMacroAt  xMacroIndex: %d \n", xMacroIndex));

	// uint16_t gAddress;
	uint8_t gKeyindex;
	uint8_t k;
	for(k = 0; k < CUSTOM_MACRO_SIZE_MAX; ++k){
		gKeyindex = pgm_read_byte(macroAddress + (CUSTOM_MACRO_SIZE_MAX * xMacroIndex) + k);
		
		if(gKeyindex > 0 && gKeyindex < 255){
			pushM(gKeyindex);
		}
	}
}

void enterFrameForCustomMacro(void){
	// timer는 최소한의 시간 체크용도로만 사용하고, 나머지 처리는 메인 loop에서 한다.
}

// 매크로 실행 중에는 다른 리포트를 하지 않도록 막는다.
// 단, 현재는 매크로에 딜레이를 적용하지 않았으므로 막을 필요는 없다.
bool isActiveCustomMacro(void){
	return false;
}

