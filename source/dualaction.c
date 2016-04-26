
#include "dualaction.h"
#include "keymap.h"
// #include "print.h"
#include "macrobuffer.h"
#include "keymapper.h"
#include "keymatrix.h"
#include "keyindex.h"
#include "keyscan.h"
#include "hardwareinfo.h"
#include "oddebug.h"

static uint8_t dualActionKeyIndex = 0;
//static uint8_t _dualActionCount = 0;
static bool _isCompounded = false;
//static bool _isActiveDualAction = false;
static uint16_t _autoDownCount = 0;

static uint8_t getDualActionCompoundKey(uint8_t keyidx);
static uint8_t getDualActionAloneKey(uint8_t keyidx);

static bool isCompounded(void)
{
	return _isCompounded;
}

void enterFrameForDualAction(void){
	/*
	 * 듀얼액션 키가 눌려진 상태로 일정 시간 (약 0.5초?)이 유지되면 일반키가 조합되지 않아도 조합 키로 적용
	 */
//	if(dualActionKeyIndex > 0 && ++_autoDownCount > 500){
	if(dualActionKeyIndex > 0 && _isCompounded == false && ++_autoDownCount > 500){
		_isCompounded = true;
		applyDualActionDownWhenIsCompounded();
	}

}

void applyDualActionDownWhenIsCompounded(void){
	 if(dualActionKeyIndex > 0 && isCompounded()){
        // 다른 키와 조합되었을 때 우선 듀얼액션키의 조합 키코드 값을 버퍼에 저장한다.
		pushKeyCodeDecorator(getDualActionCompoundKey(dualActionKeyIndex), true);

        if(isMacroInput()){
        	putKeyindex(getDualActionCompoundKey(dualActionKeyIndex), 0, 0, 1);
        }

//        dualActionKeyIndex = 0;
		_autoDownCount = 0;
    }
}

static void applyDualActionUp(void){

    if(dualActionKeyIndex > 0 && !isCompounded()){
        // 듀얼액션이 저장되어 있을 때 아무키도 눌리지 않은 리포트가 간다면 액션!
       	uint8_t gUpIdx = getDualActionAloneKey(dualActionKeyIndex);
        pushMacroKeyIndex(gUpIdx);
        pushMacroKeyIndex(gUpIdx);

        dualActionKeyIndex = 0;
		_autoDownCount = 0;

        if(isMacroInput()){
        	putKeyindex(gUpIdx, 0, 0, 1);
        	putKeyindex(gUpIdx, 0, 0, 0);
        }
    }

    // 듀얼액션 키가 모두 up 되었을 때만 active 해제;
	/*if(_dualActionCount == 0) {
        _isActiveDualAction = false;

    }*/
}

void setDualAction(uint8_t keyidx, bool isDown){
//    DBG1(0xF0, (uchar *)&isDown, 1);

    /*
     * 첫 키(modi 제외)로 듀얼액션 키를 눌러야 작동;
     *
     *
     */

	if(isDown){
//	    DBG1(0xF1, (uchar *)&keyidx, 1);
	    IF_IS_DUAL_ACTION_KEY(keyidx) {
			if (isReleaseAll() && dualActionKeyIndex == 0) { // 첫 키가 듀얼 액션 키로 눌린 상태;
//			    DBG1(0xF1, (uchar *)&dualActionKeyIndex, 5);
				dualActionKeyIndex = keyidx;
				_isCompounded = false;  // 하나만 눌렸으므로 아직은 조합 상태 아님;
			} else {
				// 듀얼 액션 키가 2개 이상 눌려짐;
			    /*
			     * 첫 듀얼 액션 키만 무조건 compound키로 작동하고,
			     * 두 번째 키들은 default키로 작동한다.
			     */
				_isCompounded = true;
			}
		} else {
			/* 듀얼 액션 키보다 다른 키가 먼저 눌린 경우,
		    *
		    * 1. alone 키가 작동되는 경우;
		    * 2. compound 키가 작동되는 경우
		    */
			_isCompounded = true;
		}
//        DBG1(0xF2, (uchar *)&_isActiveDualAction, 2);
//        DBG1(0xF2, (uchar *)&_isCompounded, 3);
	} else {
//	    DBG1(0xF3, (uchar *)&keyidx, 1);
	    IF_IS_DUAL_ACTION_KEY(keyidx) {
//			if (_dualActionCount > 0) --_dualActionCount;

			applyDualActionUp();
		} else {

		}

	}
//    DBG1(0xF4, (uchar *)&_dualActionCount, 1);
}
void clearDualAction(void)
{
//    _dualActionCount = 0;
    dualActionKeyIndex = 0;
    _isCompounded = false;
//    _isActiveDualAction = false;
}
//-------------------------------------------------------------------------------

uint8_t getDualActionDefaultKey(uint8_t xActionIndex){
    IF_IS_DUAL_ACTION_KEY(xActionIndex){
        // 조합 키가 우선이되 FN 계열의 키는 alone키로 반환
        uint8_t gIndex =  getDualActionCompoundKey(xActionIndex);
        if(isFnKey(gIndex)){
            return getDualActionAloneKey(xActionIndex);
        }
        return gIndex;
    }
    return xActionIndex;
}

// 키가 조합되었을 때는 조합 키코드를 적용한다.;
uint8_t getDualActionDownKeyIndexWhenIsCompounded(uint8_t xKeyidx, bool xForceCompounded){
//    DBG1(0xF8, (uchar *)&xKeyidx, 2);
//    DBG1(0xF1, (uchar *)&dualActionKeyIndex, 3);
    IF_IS_DUAL_ACTION_KEY(xKeyidx){
        if(xForceCompounded == true)
        {
            return getDualActionCompoundKey(xKeyidx);
        }
        else if(isCompounded())
        {
            // 처음 눌린 키만 compound
            if(dualActionKeyIndex == xKeyidx)
            {
                return getDualActionCompoundKey(xKeyidx);
            }
            else
            {
                // 나머지는 default key
                return getDualActionDefaultKey(xKeyidx);
            }
        }
    }

//    DBG1(0xF8, (uchar *)&xKeyidx, 1);

    return xKeyidx;
}

// 조합 키코드를 반환한다.
static uint8_t getDualActionCompoundKey(uint8_t xActionIndex){
//    if(xActionIndex > KEY_dualAction && xActionIndex < KEY_dualAction_end){
        xActionIndex = getExchangedKeyindex(pgm_read_byte(DUALACTION_ADDRESS + (xActionIndex - (KEY_dualAction + 1)) * 2));
//    }
    return xActionIndex;
}

// 각개 키코드 반환;
static uint8_t getDualActionAloneKey(uint8_t xActionIndex){
//    if(xActionIndex > KEY_dualAction && xActionIndex < KEY_dualAction_end){
        xActionIndex = getExchangedKeyindex(pgm_read_byte(DUALACTION_ADDRESS + 1 + (xActionIndex - (KEY_dualAction + 1)) * 2));
//    }
    return xActionIndex;
}
