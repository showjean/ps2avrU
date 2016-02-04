
// #include "print.h"

#include "keyscan.h"
#include "dualaction.h"
#include "keydownbuffer.h"
#include "keyindex.h"
#include "keymap.h"
#include "keymapper.h"
#include "keymatrix.h"
#include "fncontrol.h"
#include "bootmapper.h"
#include "esctilde.h"
#include "ledrender.h"
#include "oddebug.h"

static void scanKey(uint8_t xLayer);
static void sendKeyCodeWhenChange(uint8_t xKeyidx_not_dualaction_idx, bool xIsDown);
static void scanKeyWithDebounce(void);

static keyscan_driver_t *keyscanDriver;

void setKeyScanDriver(keyscan_driver_t *driver)
{
    keyscanDriver = driver;
}

static void sendKeyCodeWhenChange(uint8_t xKeyidx_not_dualaction_idx, bool xIsDown)
{
    (*keyscanDriver->pushKeyCodeWhenChange)(xKeyidx_not_dualaction_idx, xIsDown);
}

void pushKeyCodeDecorator(uint8_t xKeyidx, bool xIsDown){

//    if(xIsDown){
//        pushDownBuffer(getDualActionDownKeyIndexWhenIsCompounded(xKeyidx, false), xIsDown);
//    }

    sendKeyCodeWhenChange(xKeyidx, xIsDown);
}


static void putChangedKey(uint8_t xKeyidx, bool xIsDown, uint8_t xCol, uint8_t xRow){

    applyKeyDownForFullLED(xKeyidx, xCol, xRow, xIsDown);

    // 빈 키코드는 LED 반응 이외의 기능 없음;
    if(xKeyidx == KEY_NONE ) return;


	bool gFN = applyFN(xKeyidx, xCol, xRow, xIsDown);

    setDualAction(xKeyidx, xIsDown);

    // down : 조합키 판정 전에는 듀얼 액션 키의 인덱스로 전달되고
    // up : 조합 후 modi 키로 up이 되어서 key down count가 제대로 작동하지 않는다.
//    pushDownBuffer(getDualActionDownKeyIndexWhenIsCompounded(xKeyidx, false), xIsDown);

    // 듀얼 액션 키 자체로 버퍼에 저장하면?
    pushDownBuffer(xKeyidx, xIsDown);

    if(xIsDown){
        applyDualActionDownWhenIsCompounded();
    }


    // 키매핑 진행중;
    // isKeyMapping()을 쓰면 ps2에서 눌렸던 키들이 복귀 되지 않는다.
    if(isDeepKeyMapping()){
        
        putKeyindex(xKeyidx, xCol, xRow, xIsDown);

        return;
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

static uint8_t processKeyIndex(uint8_t xKeyidx, bool xPrev, bool xCur, uint8_t xCol, uint8_t xRow){

	// !(prev&&cur) : 1 && 1 이 아니고,
    // !(!prev&&!cur) : 0 && 0 이 아니고,
    // 이전 상태에서(press/up) 변화가 있을 경우;
    //if( !(prev&&cur) && !(!prev&&!cur)) {
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

        setKeyEnabled(xKeyidx, xCur);

        if(isKeyEnabled(xKeyidx) == false) return 0;

        if(xCur) {
            DBG1(0xB1, (uchar *)&xKeyidx, 1);
            putChangedKey(xKeyidx, true, xCol, xRow);
        }else{
            DBG1(0xC1, (uchar *)&xKeyidx, 1);
            putChangedKey(xKeyidx, false, xCol, xRow);
        }
    }

    return 0;

}

void scanKeyWithMacro(void){

    macro_key_t gKey;
    if(isActiveMacro()){
        if(!isEmptyM()){
          
            gKey = popMWithKey();

//            DBG1(0x1F, (uchar *)&gKey, 2);
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
    
//    uint8_t prevKeyidx;
//    uint8_t row, col, prev, cur, keyidx, result;
//    static bool _isFnPressedPrev = false;
//    static uint8_t _prevLayer = 0;
    uint8_t gLayer = getLayer();

//    uint8_t *gMatrix = getCurrentMatrix();
//    uint8_t *gPrevMatrix = getPrevMatrix();
    // ps/2 연결시 FN/FN2/NOR키의 레이어 전환시 같은 위치에 있는 다른 키코드의 키가 눌려지지만 손을 때면 눌려진 상태로 유지되는 버그 패치
    // 레이어가 변경된 경우에는 이전 레이어를 검색하여 달리진 점이 있는지 확인하여 적용;
//    DBG1(0xAA, (uchar *)&_prevLayer, 1);
//    DBG1(0xAA, (uchar *)&gLayer, 1);
//    DBG1(0xAB, (uchar *)&_isFnPressedPrev, 1);
    /*if( _isFnPressedPrev == false && (_prevLayer != gLayer)){
        for(col=0;col<COLUMNS;++col)
        {       
            for(row=0;row<ROWS;++row)
            {               
                prevKeyidx = getCurrentKeyindex(_prevLayer, row, col);
                keyidx = getCurrentKeyindex(gLayer, row, col);

                // 이전 상태에서(press/up) 변화가 있을 경우;

                prev : 1, cur : 1 = prev up, cur down
                prev : 1, cur : 0 = prev up
                prev : 0, cur : 1 = cur down
                prev : 0, cur : 0 = -

                if( prevKeyidx != keyidx && !isFnKey(prevKeyidx)) {
                    prev = gPrevMatrix[row] & BV(col);
                    cur  = gMatrix[row] & BV(col);

                    // 이전에 눌려져 있던 상태에서 layer가 변경되면 적용;
                    if(prev && cur){
                        DBG1(0xCC, (uchar *)&prevKeyidx, 1);
                        DBG1(0xCC, (uchar *)&keyidx, 1);
                        processKeyIndex(prevKeyidx, true, false, col, row);
                        result = processKeyIndex(keyidx, false, true, col, row);

                        if(result > 0)
                        {
                            break;
                        }
                    }
                }

            }
            
        }
    }
    _prevLayer = gLayer;
    _isFnPressedPrev = isFnPressed();*/

    scanKey(gLayer);
}

static void scanKey(uint8_t xLayer) {
//    DBG1(0x00, (uchar *)&xLayer, 1);

	uint8_t row, col, prev, cur, keyidx, result;

    uint8_t *gMatrix = getCurrentMatrix();
    uint8_t *gPrevMatrix = getPrevMatrix();
	for (row = 0; row < ROWS; ++row) { // check every bit on this row   
        if(gPrevMatrix[row] == 0 && gMatrix[row] == 0) continue;
        for (col = 0; col < COLUMNS; ++col) { // process all rows for key-codes
			// usb 입력은 눌렸을 때만 확인하면 되지만, 각종 FN키 조작을 위해서 업/다운을 모두 확인한다.
			prev = gPrevMatrix[row] & BV(col);
			cur  = gMatrix[row] & BV(col);
            keyidx = getCurrentKeyindex(xLayer, row, col);

/*
#ifdef ENABLE_BOOTMAPPER           
            if(isBootMapper()){
                if( prev != cur){
                    if(cur) trace(row, col);
                    break;
                }  
                continue;              
            }
#endif              
*/
            result = processKeyIndex(keyidx, prev, cur, col, row);
            if(result == 1){
                continue;
            }else if(result == 2){
                break;
            }

		}
	}

    setPrevMatrix();

    setCurrentMatrixAfter();

}
