
#include "dualaction.h"
#include "keymap.h"
// #include "print.h"
#include "macrobuffer.h"
#include "keymapper.h"
#include "keymatrix.h"
#include "keyindex.h"
#include "keyscan.h"
#include "oddebug.h"

const uint8_t PROGMEM dualActionCompoundMask[24] = {
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
const uint8_t PROGMEM dualActionAloneMask[24] = {
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
/*const bool PROGMEM dualActionCancelDefaultDown[] = {
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
};*/

static uint8_t dualActionKeyIndex = 0;
static uint8_t _dualActionCount = 0;
static uint8_t _normalKeyCount = 0;
static bool _isCompounded = false;
static bool _isActiveDualAction = false;
static uint16_t _autoDownCount = 0;

static uint8_t getDualActionCompoundKey(uint8_t keyidx);
static uint8_t getDualActionAloneKey(uint8_t keyidx);

static bool isCompounded(void)
{
	return _isCompounded;
}

void enterFrameForDualAction(void){
	/*
	 * 듀얼액션 키가 눌려진 상태로 일정 시간 (약 0.5초?)이 유지되면 일반키가 조합되지 않아도 다운 키 적용
	 */
	if(dualActionKeyIndex > 0 && ++_autoDownCount > 500){
		_isCompounded = true;
		applyDualActionDownWhenIsCancel(true);
	}

}

void applyDualActionDownWhenIsCancel(bool isDown){
	 if(dualActionKeyIndex > 0 && isCompounded()){
        // 다른 키와 조합되었을 때 우선 듀얼액션키의 down 값을 버퍼에 저장한다.
		pushKeyCodeDecorator(getDualActionCompoundKey(dualActionKeyIndex), isDown);

        if(isMacroInput()){
        	putKeyindex(getDualActionCompoundKey(dualActionKeyIndex), 0, 0, 1);
        }
        dualActionKeyIndex = 0;
		_autoDownCount = 0;
    }
}

static void applyDualActionUp(void){

    if(dualActionKeyIndex > 0 && !isCompounded()){
        // 듀얼액션이 저장되어 있을 때 아무키도 눌리지 않은 리포트가 간다면 액션!
       	uint8_t gUpIdx = getDualActionAloneKey(dualActionKeyIndex);
        pushM(gUpIdx);
        pushM(gUpIdx);
        dualActionKeyIndex = 0;
		_autoDownCount = 0;

        if(isMacroInput()){
        	putKeyindex(gUpIdx, 0, 0, 1);
        	putKeyindex(gUpIdx, 0, 0, 0);
        }
        // _upKeyindex = gUpIdx;
    }
    // 듀얼액션 키가 모두 up 되었을 때만 active 해제;
	if(_dualActionCount == 0) {
        _isActiveDualAction = false;

        // 모든 듀얼액션 키가 업 되었더라도 일반키는 눌려져 있을 수 있으므로 코멘트 처리;
        // _normalKeyCount = 0;
    }
}

void setDualAction(uint8_t keyidx, bool isDown){
//    DBG1(0xF0, (uchar *)&isDown, 1);
	if(isDown){
//	    DBG1(0xF1, (uchar *)&keyidx, 1);
		if (keyidx > KEY_dualAction && keyidx < KEY_dualAction_end) {
			++_dualActionCount;
			if (_isActiveDualAction == false && _normalKeyCount == 0) {
				dualActionKeyIndex = keyidx;
				_isActiveDualAction = true;
				_isCompounded = false;
			} else {
				// 듀얼액션이 저장되어 있을 때 아무 키나 눌리면 액션 중지;
				_isCompounded = true;
			}
		} else if (dualActionKeyIndex > 0) {
			// 듀얼액션이 저장되어 있을 때 아무 키나 눌리면 액션 중지;
			_isCompounded = true; // 듀얼액션을 취소 시키면 다음 듀얼액션 키 down일 때까지 계속 취소상태로 유지됨;
			++_normalKeyCount;
		} else if (_isActiveDualAction == false) {
			// 듀얼액션 키보다 다른 키들이 먼저 눌려진 경우 듀얼액션은 작동하지 않고 down 키값을 사용한다.
			++_normalKeyCount;  // 보통 키가 눌려진 수만큼 증가;
			_isCompounded = true;
		}
//        DBG1(0x03, (uchar *)&_isCanceledDualAction, 1);
	} else {
//	    DBG1(0xF4, (uchar *)&keyidx, 1);
		if (keyidx > KEY_dualAction && keyidx < KEY_dualAction_end) {
			if (_dualActionCount > 0)
				--_dualActionCount;
			applyDualActionUp();
		} else {
			if (_normalKeyCount > 0)
				--_normalKeyCount;
		}

/* 작동하지 않는 부분;
  		if (isReleaseAll()) {
			DBG1(0xC2, (uchar *)&_dualActionCount, 1);
			DBG1(0xC2, (uchar *)&_normalKeyCount, 1);
			_dualActionCount = 0;
			_normalKeyCount = 0;
			dualActionKeyIndex = 0;
			_isCanceledDualAction = false;
			_isActiveDualAction = false;
		}*/
	}
//    DBG1(0xF2, (uchar *)&_dualActionCount, 1);
}
void clearDualAction(void)
{
    _dualActionCount = 0;
    _normalKeyCount = 0;
    dualActionKeyIndex = 0;
    _isCompounded = false;
    _isActiveDualAction = false;
}
//-------------------------------------------------------------------------------

uint8_t getDualActionDefaultKeyForFncontrol(uint8_t xActionIndex){
    if(xActionIndex > KEY_dualAction && xActionIndex < KEY_dualAction_end){
        // 조합 키가 우선이되 FN 계열의 키는 제외;
        xActionIndex =  getDualActionCompoundKey(xActionIndex);
        if(isFnKey(xActionIndex)){
            return getDualActionAloneKey(xActionIndex);
        }
    }
    return xActionIndex;
}

// 키가 조합되었을 때는 조합 키코드를 적용한다.;
uint8_t getDualActionDownKeyIndexWhenIsCompounded(uint8_t xActionIndex, bool xForceCompounded){
	if(isCompounded() || xForceCompounded == true){
        return getDualActionCompoundKey(xActionIndex);
    }
    return xActionIndex;
}

// 조합 키코드를 반환한다.
static uint8_t getDualActionCompoundKey(uint8_t keyidx){
    if(keyidx > KEY_dualAction && keyidx < KEY_dualAction_end){
        keyidx = getExchangedKeyindex(pgm_read_byte(&dualActionCompoundMask[keyidx - (KEY_dualAction + 1)])); 
    }
    return keyidx;
}

// 각개 키보드 반환;
static uint8_t getDualActionAloneKey(uint8_t keyidx){
    if(keyidx > KEY_dualAction && keyidx < KEY_dualAction_end){
        keyidx = getExchangedKeyindex(pgm_read_byte(&dualActionAloneMask[keyidx - (KEY_dualAction + 1)])); 
    }
    return keyidx;
}
