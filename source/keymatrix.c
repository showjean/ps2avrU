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
#include "quickmacro.h"
#include "keyindex.h"
#include "dualaction.h"
#include "keydownbuffer.h"
#include "numlocklayer.h"
#include "oddebug.h"

// ROWS*COLUMNS bit matrix
static uint8_t prevMatrix[ROWS];
static uint8_t currentMatrix[ROWS];  ///< contains current state of the keyboard

#ifdef GHOST_KEY_PREVENTION
	uint8_t *ghostFilterMatrixPointer;
	uint8_t findGhostKey(void);
#endif

/* ------------------------------------------------------------------------- */
#define DEBOUNCE_MAX    10  //4
static uint8_t debounce; // DEBOUNCE_MAX + 3, debounceMAX 보다 크게 설정하여 플러깅시 all release가 작동되는 것을 방지;
static bool _isReleaseAll = true;
static uint8_t __pressedFnIndex = LAYER_NOTHING;

//static uint8_t _currentLayer = LAYER_NOTHING;



/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  global ----------------------------- */
/* ------------------------------------------------------------------------- */
void initMatrix(void){
	
	delegateInitMatrixDevice();

	debounce = DEBOUNCE_MAX + 3;

	clearFnPosition();

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

static uint8_t fnCol;
static uint8_t fnRow;
bool isFnPosition(uint8_t xCol, uint8_t xRow)
{
    if(xCol == fnCol && xRow == fnRow)
    {
        return true;
    }
    return false;
}
void clearFnPosition(void)
{
    fnCol = 127;    // must bigger then COLUMNS, ROWS
    fnRow = 127;
}
void setFnPressed(uint8_t xFnIndex)
{
    if(xFnIndex == KEY_FN){
        __pressedFnIndex = LAYER_FN;
    }else if(xFnIndex == KEY_FN2){
        __pressedFnIndex = LAYER_FN2;
    }else if(xFnIndex == KEY_FN3){
        __pressedFnIndex = LAYER_FN3;
    }else if(xFnIndex == KEY_NOR){
        __pressedFnIndex = LAYER_NORMAL;
    }else{
        __pressedFnIndex = LAYER_NOTHING;
    }
//    DBG1(0x34, (uchar *)&__pressedFnIndex, 1);
}

bool isFnPressed(void)
{
    return __pressedFnIndex != LAYER_NOTHING;
}

uint8_t getCurrentFnLayer(void)
{
    if(getNumlockLayer())
    {
        return getNumlockLayer();
    }
    else if(getBeyondFN())
    {
        return getBeyondFN();
    }
    else
    {
        return LAYER_NORMAL;
    }
}

uint8_t getLayer(void) {
	uint8_t col, row, keyidx, cur, gLayer, fnScanLayer;

	/*
	FN이 작동되는 상황 정리;

	- 각각 키가 down/up 될 때, fn 상태에 따라서 적용된다.

	- 가장 자연스럽게 처리된다고 보면 됨;

	*/
	if(__pressedFnIndex != LAYER_NOTHING) return __pressedFnIndex;

	fnScanLayer = getCurrentFnLayer();

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

				if(gLayer != LAYER_NOTHING){

				    fnCol = col;
				    fnRow = row;

					return gLayer;
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
    setPrevMatrix();
	setReleaseAll();

	if(isReleaseAll())
	{
        clearDualAction();
	}

}


#endif
