
// #include "print.h"

#include "keyscan.h"
#include "dualaction.h"
#include "keydownbuffer.h"
#include "keyindex.h"
#include "keymap.h"
#include "quickmacro.h"
#include "keymatrix.h"
#include "fncontrol.h"
#include "bootmapper.h"
#include "esctilde.h"
#include "ledrender.h"
#include "oddebug.h"

static void sendKeyCodeWhenChange(uint8_t xKeyidx_not_dualaction_idx, bool xIsDown);
static void scanKeyWithDebounce(void);

static keyscan_driver_t *keyscanDriver;

static uint8_t oldDownedMatrix[LAYERS][ROWS];

void setKeyScanDriver(keyscan_driver_t *driver)
{
    keyscanDriver = driver;
}

static void sendKeyCodeWhenChange(uint8_t xKeyidx_not_dualaction_idx, bool xIsDown)
{
    (*keyscanDriver->pushKeyCodeWhenChange)(xKeyidx_not_dualaction_idx, xIsDown);
}

void pushKeyCodeDecorator(uint8_t xKeyidx, bool xIsDown){

    sendKeyCodeWhenChange(xKeyidx, xIsDown);
}


static void putChangedKey(uint8_t xKeyidx, bool xIsDown, uint8_t xCol, uint8_t xRow){

    applyKeyDownForFullLED(xKeyidx, xCol, xRow, xIsDown);

    // 빈 키코드는 LED 반응 이외의 기능 없음;
    if(xKeyidx == KEY_NONE ) return;

    // fn 키가 눌렸을 경우 해당 위치의 키는 무시한다.
    uint8_t gLayer, gKeyIndex, gFnIndex;

    gLayer = getCurrentFnLayer();
    DBG1(0xC2, (uchar *)&gLayer, 1);

    if(isFnPosition(xCol, xRow) /* -> */ && (xIsDown || (!xIsDown && isFnPressed())) /* <- */)
        // 키 입력 동안 레이어 토글이 변경될 경우 FN 키와 같은 위치의 키들은 up 신호가 무시 되어 계속 눌려진 상태가 된다. 이를 패치.
        // 이 경우 fnPressed가 false 상태로 진행되므로 이를 확인
    {
        // 현재 레이어에서 눌린 FN키가 듀얼 액션 키이면, 변경된 레이어의 키를 듀얼 액션 키로 강제 치환시켜서 진행
        gKeyIndex = getCurrentKeyindex(gLayer, xRow, xCol);

        // fn key는 하나만 눌릴 수 있도록 처리;
        if(xIsDown)
        {
            // dual action key라면 FN을 추출;
            gFnIndex = getDualActionDownKeyIndexWhenIsCompounded(gKeyIndex, true);
//            DBG1(0x32, (uchar *)&gFnIndex, 1);
            setFnPressed(gFnIndex);
        }
        else
        {
            setFnPressed(KEY_NONE);
            clearFnPosition();
        }

        IF_IS_DUAL_ACTION_KEY(gKeyIndex)
        {
            xKeyidx = gKeyIndex;
        }
        else
        {
            return;
        }
    }


	bool gFN = applyFN(xKeyidx, xCol, xRow, xIsDown);

    setDualAction(xKeyidx, xIsDown);

    // 듀얼 액션 키 자체로 버퍼에 저장하면?
    pushDownBuffer(xKeyidx, xIsDown);

    if(xIsDown){
        applyDualActionDownWhenIsCompounded();
    }

    // 키매핑 진행중;
    // isKeyMapping()을 쓰면 ps2에서 눌렸던 키들이 복귀 되지 않는다.
    if(isDeepKeyMapping()){
        
        xKeyidx = putKeyindex(xKeyidx, xCol, xRow, xIsDown);

        if(xKeyidx == KEY_NONE) return;
    }

    // fn키를 키매핑에 적용하려면 위치 주의;
    if(gFN == false) return;

    if(isFnKey(xKeyidx)) return;
            
    if(xIsDown && applyMacro(xKeyidx)) {
        // 매크로 실행됨;
        return;
    }

    // shift가 눌려있고 ESC to ~ 옵션이 on 이라면 ESC를 `키로 변환한다.
    xKeyidx = getEscToTilde(xKeyidx, xIsDown);
    xKeyidx = getDualActionDownKeyIndexWhenIsCompounded(xKeyidx, false);

    sendKeyCodeWhenChange(xKeyidx, xIsDown);
	
}

static uint8_t processKeyIndex(uint8_t xLayer, bool xPrev, bool xCur, uint8_t xCol, uint8_t xRow ){

    uint8_t keyidx, i;

    if( xPrev != xCur ) {

#ifdef ENABLE_BOOTMAPPER
        if (isBootMapper()) {
            if (xCur) {
                trace(xRow, xCol);
                return 2;
            }
            return 1;
        }
#endif

        keyidx = getCurrentKeyindex(xLayer, xRow, xCol);

        setKeyEnabled(keyidx, xCur);

        if(isKeyEnabled(keyidx) == false) return 0;

        if(xCur) {
//            DBG1(0xB1, (uchar *)&keyidx, 1);

            // mark for downed layer
            oldDownedMatrix[xLayer][xRow] |= BV(xCol);

            putChangedKey(keyidx, true, xCol, xRow);
        }else{

            // check layer for up
            i = 0;
            for (  ; i < LAYERS ; ++i)
            {
                if(oldDownedMatrix[i][xRow] & BV(xCol))
                {
                    xLayer = i;
                    break;
                }
            }

            keyidx = getCurrentKeyindex(xLayer, xRow, xCol);
            oldDownedMatrix[xLayer][xRow] &= ~BV(xCol);

            DBG1(0xC0, (uchar *)&xLayer, 1);
            DBG1(0xC1, (uchar *)&keyidx, 1);
            putChangedKey(keyidx, false, xCol, xRow);
        }
    }

    return 0;

}

void scanKeyWithMacro(void){

    macro_key_t gKey;
    if(isActiveMacro()){
        if(!isEmptyMacroKeyIndex()){
          
            gKey = popMacroKey();

            if(gKey.mode == MACRO_KEY_DOWN){    // down
                sendKeyCodeWhenChange(gKey.keyindex, true);
                
            }else{  // up
                // 모디키가 눌려져 있다면 그 상태를 유지;
                if (gKey.keyindex > KEY_Modifiers && gKey.keyindex < KEY_Modifiers_end) {
                    if(getModifierDownBuffer() & getModifierBit(gKey.keyindex)){                     
                        goto PASS_MODI;
                    }
                }
                sendKeyCodeWhenChange(gKey.keyindex, false);
             
            }
        }

    }

PASS_MODI:

    scanKeyWithDebounce();
}

static void scanKeyWithDebounce(void) {
    
    // debounce cleared and changed
    if(!setCurrentMatrix()) return;

	uint8_t row, col, prev, cur, result;
    uint8_t gLayer = getLayer();

    DBG1(0x33, (uchar *)&gLayer, 1);

    uint8_t *gMatrix = getCurrentMatrix();
    uint8_t *gPrevMatrix = getPrevMatrix();
	for (row = 0; row < ROWS; ++row) { // check every bit on this row   
        if(gPrevMatrix[row] == 0 && gMatrix[row] == 0) continue;
        for (col = 0; col < COLUMNS; ++col) { // process all rows for key-codes

			// usb 입력은 눌렸을 때만 확인하면 되지만, 각종 FN키 조작을 위해서 업/다운을 모두 확인한다.
			prev = gPrevMatrix[row] & BV(col);
			cur  = gMatrix[row] & BV(col);

            result = processKeyIndex(gLayer, prev, cur, col, row);
            if(result == 1){
                continue;
            }else if(result == 2){
                break;
            }

		}
	}

    setCurrentMatrixAfter();

}
