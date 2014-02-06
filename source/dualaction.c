
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
static uint8_t _dualActionCount = 0;
static uint8_t _normalKeyCount = 0;
static bool _isCanceledDualAction = false;
static bool _isActiveDualAction = false;

static uint8_t getDualActionMaskUp(uint8_t keyidx);

static bool isCanceledDualAction(void)
{
	return _isCanceledDualAction;
}

void applyDualActionDownWhenIsCancel(uint8_t (*func)(uint8_t, bool), bool isDown){
	 if(dualActionKeyIndex > 0 && isCanceledDualAction()){
        // 듀얼액션 활성화 후 다른 키가 눌려 취소되었을 때 우선 듀얼액션키의 down 값을 버퍼에 저장한다.
        (*func)(getDualActionMaskDown(dualActionKeyIndex), isDown);

        if(isMacroInput()){
        	// DEBUG_PRINT(("applyDualActionDown isMacroInput : %d\n", dualActionKeyIndex)); 
        	putKeyindex(getDualActionMaskDown(dualActionKeyIndex), 0, 0, 1);
        }
        dualActionKeyIndex = 0;
    }
}

static void applyDualActionUp(void){

	// DEBUG_PRINT(("applyDualAction \n"));

    if(dualActionKeyIndex > 0 && !isCanceledDualAction()){
        // 듀얼액션이 저장되어 있을 때 아무키도 눌리지 않은 리포트가 간다면 액션!
       	uint8_t gUpIdx = pgm_read_byte(&dualActionMaskUp[dualActionKeyIndex - (KEY_dualAction + 1)]);
        pushM(gUpIdx);
        pushM(gUpIdx);
        dualActionKeyIndex = 0;
        // DEBUG_PRINT(("applyDualActionUp : %d\n", gUpIdx)); 

        if(isMacroInput()){
        	// DEBUG_PRINT(("applyDualActionUp isMacroInput : %d\n", gUpIdx)); 
        	putKeyindex(gUpIdx, 0, 0, 1);
        	putKeyindex(gUpIdx, 0, 0, 0);
        }
    }
    // 듀얼액션 키가 모두 up 되었을 때만 active 해제;    
	if(_dualActionCount == 0) {
        _isActiveDualAction = false;

        // 모든 듀얼액션 키가 업 되었더라도 일반키는 눌려져 있을 수 있으므로 코멘트 처리;
        // _normalKeyCount = 0;
    }
}

void setDualAction(uint8_t keyidx, bool isDown){
	if(isDown){
		if(keyidx > KEY_dualAction && keyidx < KEY_dualAction_end){
			++_dualActionCount;
		  	if(_isActiveDualAction == false && _normalKeyCount == 0){
		        dualActionKeyIndex = keyidx;
		        _isActiveDualAction = true;
				_isCanceledDualAction = false;
				// DEBUG_PRINT(("dualActionKeyIndex: %d \n", dualActionKeyIndex));
			}else{
	        	// 듀얼액션이 저장되어 있을 때 아무 키나 눌리면 액션 중지;    
				_isCanceledDualAction = true;
			}
	    }else if(dualActionKeyIndex > 0){
	        // 듀얼액션이 저장되어 있을 때 아무 키나 눌리면 액션 중지;    
	        // DEBUG_PRINT(("dualActionKeyIndex cancel : %d\n", dualActionKeyIndex)); 
	        _isCanceledDualAction = true;       // 듀얼액션을 취소 시키면 다음 듀얼액션 키 down일 때까지 계속 취소상태로 유지됨;
            ++_normalKeyCount;
        }else if(_isActiveDualAction == false){
            // 듀얼액션 키보다 다른 키들이 먼저 눌려진 경우 듀얼액션은 작동하지 않고 down 키값을 사용한다.
            ++_normalKeyCount;  // 보통 키가 눌려진 수만큼 증가;
            _isCanceledDualAction = true; 
	    }
	}else{
		if(keyidx > KEY_dualAction && keyidx < KEY_dualAction_end){	
			if(_dualActionCount > 0) --_dualActionCount;
			applyDualActionUp();
		}else{
            if(_normalKeyCount > 0) --_normalKeyCount;
        }

        if(isReleaseAll()){
            _dualActionCount = 0;
            dualActionKeyIndex = 0;
            _normalKeyCount = 0;
            _isCanceledDualAction = false;
            _isActiveDualAction = false;
        }
	}
    // DEBUG_PRINT(("_dualActionCount :::: %d\n", _dualActionCount)); 
}
uint8_t getDualActionDefaultKeyIndexWhenIsCancel(uint8_t xActionIndex){
    if(isCanceledDualAction()){
        bool gIsDefaultDown = pgm_read_byte(&dualActionCancelDefaultDown[xActionIndex - (KEY_dualAction + 1)]);
        if(gIsDefaultDown){
            return getDualActionMaskDown(xActionIndex); 
        }else{
            return getDualActionMaskUp(xActionIndex); 
        }
    }
    return xActionIndex;
}

// 듀얼액션 취소되었을 때는 다운 키코드를 적용한다.;
uint8_t getDualActionDownKeyIndexWhenIsCancel(uint8_t xActionIndex){
	if(isCanceledDualAction()){
        return getDualActionMaskDown(xActionIndex);         
    }
    return xActionIndex;
}
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
