
#include "dualaction.h"
#include "keymap.h"
#include "print.h"
#include "macrobuffer.h"
#include "keymapper.h"
#include "keymatrix.h"
#include "quickswap.h"

const uint8_t PROGMEM dualActionMaskDown[] = {
    KEY_FN, // FN
    KEY_FN, // FN
    KEY_FN2, // FN2
    KEY_FN2, // FN2
    KEY_RSHIFT,
    KEY_RSHIFT,    
    KEY_LSHIFT,
    KEY_LSHIFT,
    KEY_RALT,
    KEY_RALT,
    KEY_LALT,
    KEY_LALT,
    KEY_RCTRL,
    KEY_RCTRL,
    KEY_LCTRL,
    KEY_LCTRL,
    KEY_FN,    
    KEY_FN2,
    KEY_FN3,
    KEY_FN3,
    KEY_FN3,
    KEY_FN,    
    KEY_FN2,
    KEY_FN3

};
const uint8_t PROGMEM dualActionMaskUp[] = {
    KEY_HANGLE, // hangle
    KEY_HANJA,  // hanja
    KEY_HANGLE, // hangle
    KEY_HANJA,  // hanja
    KEY_HANGLE,
    KEY_HANJA,  // hanja    
    KEY_HANGLE,
    KEY_HANJA,
    KEY_HANGLE,
    KEY_HANJA,
    KEY_HANGLE,
    KEY_HANJA,
    KEY_HANGLE,
    KEY_HANJA,
    KEY_HANGLE,
    KEY_HANJA,
    KEY_CAPS,
    KEY_CAPS,
    KEY_HANGLE,
    KEY_HANJA,
    KEY_CAPS,
    KEY_APPS,
    KEY_APPS,
    KEY_APPS
};
// 듀얼액션 취소시 출력할 코드값이 "다운"쪽 설정 값인지 결정;
const bool PROGMEM dualActionCancelDefaultDown[] = {
    true, 
    true,  
    true, 
    true, 
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    false,
    false,
    true,
    true,
    false,
    true,
    true,
    true
};

static uint8_t dualActionKeyIndex = 0;
static uint8_t _keyCount = 0;
// static uint8_t _normalKeyCount = 0;
static bool _isCanceledDualAction = false;
static bool _isActiveDualAction = false;

static uint8_t getDualActionMaskUp(uint8_t keyidx);

static bool isCanceledDualAction(void)
{
	return _isCanceledDualAction;
}

static void applyDualActionUp(void){

	// DEBUG_PRINT(("applyDualAction \n"));

    if(dualActionKeyIndex > 0 && !isCanceledDualAction()){
        // 듀얼액션이 저장되어 있을 때 아무키도 눌리지 않은 리포트가 간다면 액션!
       	uint8_t gUpIdx = pgm_read_byte(&dualActionMaskUp[dualActionKeyIndex - (KEY_dualAction + 1)]);
        pushM(gUpIdx);
        pushM(gUpIdx);
        dualActionKeyIndex = 0;

        if(isMacroInput()){
        	putKeyindex(gUpIdx, 0, 0, 1);
        	putKeyindex(gUpIdx, 0, 0, 0);
        }
    }    
}

void setDualAction(uint8_t keyidx, bool isDown){

// applyDualActionUp이 한번 뿐이 실행되지 않는다.

	if(isDown){
        ++_keyCount;
        if(_isActiveDualAction){
            _isCanceledDualAction = true; 
        }else if(keyidx > KEY_dualAction && keyidx < KEY_dualAction_end){
            dualActionKeyIndex = keyidx;
			_isActiveDualAction = true;
            _isCanceledDualAction = false;
	    }
	}else{
        if(_keyCount > 0) --_keyCount;
		if(keyidx > KEY_dualAction && keyidx < KEY_dualAction_end){				
			applyDualActionUp();
		}

// isReleaseAll()은 이 후에 세팅된다.
        
        if(_keyCount == 0){
            _isCanceledDualAction = false;
            _isActiveDualAction = false;
        }
	}
    // DEBUG_PRINT(("_keyCount :::: %d\n", _keyCount)); 
}

uint8_t getDualActionDefaultKey(uint8_t xActionIndex){
    if(xActionIndex > KEY_dualAction && xActionIndex < KEY_dualAction_end){
        bool gIsDefaultDown = pgm_read_byte(&dualActionCancelDefaultDown[xActionIndex - (KEY_dualAction + 1)]);
        if(gIsDefaultDown){
            return getDualActionMaskDown(xActionIndex); 
        }else{
            return getDualActionMaskUp(xActionIndex); 
        }
    }
    return xActionIndex;
}
/*uint8_t getDualActionDefaultKeyIndexWhenIsCancel(uint8_t xActionIndex){
    if(isCanceledDualAction()){
        return getDualActionDefaultKey(xActionIndex);
    }
    return xActionIndex;
}*/

// 듀얼액션 취소되었을 때는 다운 키코드를 적용한다.;
/*uint8_t getDualActionDownKeyIndexWhenIsCancel(uint8_t xActionIndex){
	if(isCanceledDualAction()){
        return getDualActionMaskDown(xActionIndex);         
    }
    return xActionIndex;
}*/
// 듀얼액션 키인덱스라면 다운 키코드를 반환한다.
uint8_t getDualActionMaskDown(uint8_t keyidx){
    if(keyidx > KEY_dualAction && keyidx < KEY_dualAction_end){
        keyidx = getQuickSwapKeyindex(pgm_read_byte(&dualActionMaskDown[keyidx - (KEY_dualAction + 1)])); 
    }
    return keyidx;
}
static uint8_t getDualActionMaskUp(uint8_t keyidx){
    if(keyidx > KEY_dualAction && keyidx < KEY_dualAction_end){
        keyidx = getQuickSwapKeyindex(pgm_read_byte(&dualActionMaskUp[keyidx - (KEY_dualAction + 1)])); 
    }
    return keyidx;
}
