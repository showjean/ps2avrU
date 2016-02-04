#ifndef KEYMATRIX_INC_C
#define KEYMATRIX_INC_C

#include "timerinclude.h"
#include "main.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#ifdef DEBUG_QUICK_BOOTLOADER
	#include "boot.h"
#endif

#include "keymatrix.h"
#include "keymap.h"
#include "fncontrol.h"
//#include "lazyfn.h"
#include "keymapper.h"
#include "keyindex.h"
#include "dualaction.h"
#include "keydownbuffer.h"
#include "oddebug.h"

// ROWS*COLUMNS bit matrix
static uint8_t prevMatrix[ROWS];
static uint8_t currentMatrix[ROWS];  ///< contains current state of the keyboard

#ifdef GHOST_KEY_PREVENTION
	uint8_t *ghostFilterMatrixPointer;
	uint8_t findGhostKey(void);
#endif

/* ------------------------------------------------------------------------- */
#define DEBOUNCE_MAX 4
static uint8_t debounce = 7;	// DEBOUNCE_MAX + 3, debounceMAX 보다 크게 설정하여 플러깅시 all release가 작동되는 것을 방지;
static bool _isReleaseAll = true;

static uint8_t _currentLayer = LAYER_NOTHING;



/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  global ----------------------------- */
/* ------------------------------------------------------------------------- */
void initMatrix(void){
	
	delegateInitMatrixDevice();

#ifdef GHOST_KEY_PREVENTION	
	ghostFilterMatrixPointer = currentMatrix;
#endif
}

void clearMatrix(void){
	memset(prevMatrix, 0, ROWS);
	memset(currentMatrix, 0, ROWS);
}

static void setReleaseAll(void){
	uint8_t row;
	_isReleaseAll = true;
	for(row=0;row<ROWS;row++) {
		if(currentMatrix[row] > 0){
			_isReleaseAll = false;
			break;
		}
	}

}

bool isReleaseAll(void){	
	return _isReleaseAll;
}

uint8_t getLayer(void) {
	uint8_t col, row, keyidx, cur, gLayer;

	static uint8_t fnScanLayer = 0;

	/*
	FN이 작동되는 상황 정리;
	- 첫키로 FN이 눌려야 한다. 이미 다른 키가 눌려있다면 작동 안 함;

	- 작동이 된 후에는 모든 키가 release 되는 순간까지 layer를 유지 시킨다.
	(즉, 모든 키가 release 되고 1프레임 후에 작동 해제 되어야한다. 
	ps2의 경우 제일 마지막 키의 release값을 처리해야하기 때문에.)
	*/

	if(_currentLayer != LAYER_NOTHING) {
		return _currentLayer;
	}

	if(getBeyondFN()) {
	    fnScanLayer = getBeyondFN();
    }else{
        fnScanLayer = LAYER_NORMAL;
    }

	// 다른 키가 눌려있는 경우에는 FN키 작동하지 않도록;
	if(!isReleaseAll())
	{
	    return fnScanLayer;
	}

    gLayer = LAYER_NOTHING;

    uint8_t *gMatrix = getCurrentMatrix();

	for( row = 0 ; row < ROWS ; ++row ){

		if(gMatrix[row] == 0) continue;

		for( col = 0 ; col < COLUMNS ; ++col ){

			cur  = gMatrix[row] & BV(col);

			if(cur){
				keyidx = getCurrentKeyindex(fnScanLayer, row, col);

			    /**
			     * Extra FN 키가 눌렸을 경우에는 dual action 의 FN 키들은 반영하지 않는다.
			     */
				if(isDownExtraFn() == false)
				{
				    keyidx = getDualActionDownKeyIndexWhenIsCompounded(keyidx, true);   // dual action 의 FN계열 키는 무조건 검출;
				}
				
				if(keyidx == KEY_FN){
					gLayer = LAYER_FN;
				}else if(keyidx == KEY_FN2){
					gLayer = LAYER_FN2;
				}else if(keyidx == KEY_FN3){
					gLayer = LAYER_FN3;		
				}else if(keyidx == KEY_NOR){
					if(fnScanLayer == LAYER_FN2 || fnScanLayer == LAYER_FN3){	// fn2/3에서만 작동;
						// _fnScanLayer은 2를 유지하면서 스캔할 레이어는 0으로 반환;
					    gLayer = LAYER_NORMAL;
					}
				}

//				DBG1(0x02, (uchar *)&gLayer, 1);

				if(gLayer != LAYER_NOTHING){

				    // _fnScanLayer은 유지하면서 스캔할 레이어는 gLayer로 반환;
                    _currentLayer = gLayer;

					return _currentLayer;
				}
			}
		}
	}

	return fnScanLayer;
}

#ifdef GHOST_KEY_PREVENTION
uint8_t findGhostKey(void){
	uint8_t col, row, gCol, gRow;
	for(gRow=0;gRow<ROWS;gRow++)
	{		
		if(currentMatrix[gRow] == 0) continue;

		for (gCol = 0; gCol < COLUMNS; ++gCol)
		{			
			if(currentMatrix[gRow] & BV(gCol)){
				// matrix가 set 되었다.
				// 현재 row보다 낮은 위치에 또 다른 set이 있는지 확인;
				for(col = gCol+1; col < COLUMNS; ++col)
				{
					if(currentMatrix[gRow] & BV(col)){
						//또다른 col에 set이 있다.
						// 그렇다면, 같은 col의 하위 row에 또 다른 set이 있는지 확인;
						for(row = gRow+1; row < ROWS; ++row)
						{						
							if((currentMatrix[row] & (1<<col))> 0) {
								// DEBUG_PRINT(("GHOST_KEY_PREVENTION \n"));
								// 고스트 키 확인;
								return 1;
							}
						}
					}
				}
			}
			
		}
	}
	return 0;
}
#endif

uint8_t getLiveMatrix(void){
	
	uint8_t isModified = 0;

	delegateGetLiveMatrix(currentMatrix, &isModified);

	if(isModified){
		debounce=0;
	}else if(debounce<100){	// to prevent going over limit of int
		// 키 입력에 변화가 없다면 99에서 멈춰서 0을 계속 반환하게 된다. 때문에, 키 변화없을때는 키코드 갱신없음;
		debounce++;
	}

	if(debounce != DEBOUNCE_MAX){
		return 0;
	}

#ifdef GHOST_KEY_PREVENTION
	// ghost-key prevention
	// col에 2개 이상의 입력이 있을 경우, 입력이 있는 row에는 더이상 입력을 허용하지 않는다.	
	
	if(findGhostKey() > 0){
		ghostFilterMatrixPointer = prevMatrix;
	}else{
		ghostFilterMatrixPointer = currentMatrix;
	}

#endif

	return 1;
}

uint8_t *getCurrentMatrix(void){

#ifdef GHOST_KEY_PREVENTION
	return ghostFilterMatrixPointer;
#else
	return currentMatrix;
#endif

}

static void setPrevMatrix(void){
	memcpy(prevMatrix, getCurrentMatrix(), ROWS);
}

uint8_t *getPrevMatrix(void){
	return prevMatrix;
}

// return debounce clear
uint8_t setCurrentMatrix(void){	
	return getLiveMatrix();
}

// 매트릭스에 관련된 모든 처리가 끝난 후 실행 된다.
void setCurrentMatrixAfter(void){
//    DBG1(0x11, (uchar *)&prevMatrix, ROWS);
//    DBG1(0x11, (uchar *)&currentMatrix, ROWS);
    setPrevMatrix();
	setReleaseAll();

	// 모든 키가 release되면 FN 해제;
	if(isReleaseAll())
	{
		_currentLayer = LAYER_NOTHING;
        clearDualAction();
	}

}


#endif
