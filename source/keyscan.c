
#include "print.h"

#include "keyscan.h"
#include "dualaction.h"
#include "keydownbuffer.h"
#include "keyindex.h"
#include "keymap.h"
#include "keymapper.h"
#include "keymatrix.h"
#include "fncontrol.h"
#include "bootmapper.h"


static keyscan_driver_t *keyscanDriver;

void setKeyScanDriver(keyscan_driver_t *driver)
{
    keyscanDriver = driver;
}

static uint8_t pushKeyCodeDecorator(uint8_t xKeyidx, bool xIsDown){
    
    if(xIsDown){ 
        // 듀얼액션 취소되었을 때는 down 키코드를 적용한다.;       
        pushDownBuffer(getDualActionDownKeyIndexWhenIsCancel(xKeyidx));
    }
    
	return (*keyscanDriver->pushKeyCode)(xKeyidx, xIsDown);	
}

static uint8_t putChangedKey(uint8_t xKeyidx, bool xIsDown, uint8_t xCol, uint8_t xRow){

	bool gFN = applyFN(xKeyidx, xCol, xRow, xIsDown);

    if(xIsDown && xKeyidx != KEY_NONE){
        applyDualActionDownWhenIsCancel(pushKeyCodeDecorator, true);
    }

    // 키매핑 진행중;
    // isKeyMapping()을 쓰면 ps2에서 눌렸던 키들이 복귀 되지 않는다.
    if(isDeepKeyMapping()){
        // DEBUG_PRINT(("putKey xKeyidx: %d, xIsDown: %d, xCol: %d, xRow: %d \n", xKeyidx, xIsDown, xCol, xRow));
        
        putKeyindex(xKeyidx, xCol, xRow, xIsDown);

        return 0;
    }
            
    if(xIsDown && applyMacro(xKeyidx)) {
        // 매크로 실행됨;
        return 0;
    }

    // fn키를 키매핑에 적용하려면 위치 주의;
    if(gFN == false) return 0;

    
	(*keyscanDriver->pushKeyCodeWhenChange)(xKeyidx, xIsDown);
	
    return 1;
}

uint8_t processKeyIndex(uint8_t xKeyidx, bool xPrev, bool xCur, uint8_t xCol, uint8_t xRow){

    uint8_t gRetval = 1; 

    // !(prev&&cur) : 1 && 1 이 아니고, 
    // !(!prev&&!cur) : 0 && 0 이 아니고, 
    // 이전 상태에서(press/up) 변화가 있을 경우;
    //if( !(prev&&cur) && !(!prev&&!cur)) {                
    if( xPrev != xCur ) { 
        setKeyEnabled(xKeyidx, xCur);

        if(isKeyEnabled(xKeyidx) == false) return 0;   

        if(xCur) {
            // DEBUG_PRINT(("key xKeyidx : %d 1\n", xKeyidx));
            gRetval = putChangedKey(xKeyidx, true, xCol, xRow);
        }else{
            // DEBUG_PRINT(("key xKeyidx : %d 0\n", xKeyidx));
            gRetval = putChangedKey(xKeyidx, false, xCol, xRow);
        }
    }

    if(gRetval == 0) return 0;

    // usb는 눌렸을 때만 버퍼에 저장한다.
    if(xCur){
        if(isKeyEnabled(xKeyidx) == false) return 0;  
        //DEBUG_PRINT(("key down!!! xKeyidx : %d , reportIndex : %d \n", xKeyidx, reportIndex));
        pushDownBuffer(getDualActionDownKeyIndexWhenIsCancel(xKeyidx));
        
        gRetval |= (*keyscanDriver->pushKeyCodeWhenDown)(xKeyidx, true);
    }  
    return gRetval;     
}

uint8_t scanKey(uint8_t xLayer) {

    uint8_t retval = 0;

	uint8_t row, col, prev, cur, keyidx;
    uint8_t gFN; 
    // uint8_t gResultPutKey = 1;
	uint8_t gLayer = xLayer; 

    DEBUG_PRINT(("gLayer  : %d \n", gLayer)); 

    uint8_t *gMatrix = getCurrentMatrix();
    uint8_t *gPrevMatrix = getPrevMatrix();
	for (col = 0; col < COLUMNS; ++col) { // process all rows for key-codes
		for (row = 0; row < ROWS; ++row) { // check every bit on this row   
			// usb 입력은 눌렸을 때만 확인하면 되지만, 각종 FN키 조작을 위해서 업/다운을 모두 확인한다.
			prev = gPrevMatrix[row] & BV(col);
			cur  = gMatrix[row] & BV(col);
            keyidx = getCurrentKeyindex(gLayer, row, col);	   		
			gFN = 1;

#ifdef ENABLE_BOOTMAPPER           
            if(isBootMapper()){
                if( prev != cur){
                    if(cur) trace(row, col);
                    break;
                }  
                continue;              
            }
#endif              
            gFN = processKeyIndex(keyidx, prev, cur, col, row);	
            if(gFN == 0)continue;
		}
	}

	retval |= 0x01; // must have been a change at some point, since debounce is done
	
    setPrevMatrix();

    setCurrentMatrixAfter();

    // if(gResultPutKey == 0) return 0;
	
    return retval;
}
