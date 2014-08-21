#include "timerinclude.h"
#include "main.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#ifdef DEBUG_QUICK_BOOTLOADER
    #include <avr/wdt.h>
#endif
#include "keymatrix.h"
#include "keymap.h"
#include "fncontrol.h"
#include "lazyfn.h"
#include "keymapper.h"
#include "keyindex.h"
#include "dualaction.h"
#include "i2c/i2c.h"        // include i2c support
#include "oddebug.h"

// register addresses (see "mcp23018.md")
#define IODIRA 0x00  // i/o direction register
#define IODIRB 0x01
#define GPPUA  0x0C  // GPIO pull-up resistor register
#define GPPUB  0x0D
#define GPIOA  0x12  // general purpose i/o port register (write modifies OLAT)
#define GPIOB  0x13
#define OLATA  0x14  // output latch register
#define OLATB  0x15

#define TW_READ		1
#define TW_WRITE	0

#define MCP23018_TWI_ADDRESS 0b0100000

// TWI aliases
#define TWI_ADDR_WRITE ( (MCP23018_TWI_ADDRESS<<1) | TW_WRITE )
#define TWI_ADDR_READ  ( (MCP23018_TWI_ADDRESS<<1) | TW_READ  )

#define MCP_ROWS_START	9

uint8_t mcp23018_init(void);

/*
 * MCP23018 Pin Assignments (from ergodox)

     power_negative  Vss(GND) +01---.---28o NC
                           NC o02       27o GPA7   column_7
              row_9      GPB0 +03       26+ GPA6   column_6
             row_10      GPB1 +04       25+ GPA5   column_5
             row_11      GPB2 +05       24+ GPA4   column_4
             row_12      GPB3 +06       23+ GPA3   column_3
             row_13      GPB4 +07       22+ GPA2   column_2
             row_14      GPB5 +08       21+ GPA1   column_1
             row_15      GPB6 o09       20+ GPA0   column_0
             row_16      GPB7 o10       19o INTA
     power_positive  Vdd(Vcc) +11       18o INTB
                I2C       SCL +12       17o NC
                I2C       SDA +13       16+ RESET  = Vdd(Vcc)
                           NC o14-------15+ ADDR   = Vss(GND)
 *
 */

// 17*8 bit matrix
static uint8_t prevMatrix[ROWS];
static uint8_t currentMatrix[ROWS];  ///< contains current state of the keyboard

#ifdef GHOST_KEY_PREVENTION
    uint8_t *ghostFilterMatrixPointer;
    uint8_t findGhostKey(void);
#endif

uint8_t _fnScanLayer = 0;


/* ------------------------------------------------------------------------- */
static uint8_t debounceMAX;
static uint8_t debounce;    // debounceMAX 보다 크게 설정하여 플러깅시 all release가 작동되는 것을 방지;
static bool _isReleaseAll = true;
static bool _isReleaseAllPrev = true;
static bool _isFnPressed = false;
static uint8_t _currentLazyLayer = 0;


uint8_t mcp23018_init(void) {
	uint8_t ret;
	uint8_t data[3];
	// set pin direction
	// - unused  : input  : 1
	// - input   : input  : 1
	// - driving : output : 0
	data[0] = IODIRA;
	data[1] = 0b00000000;  // IODIRA
	data[2] = (0b11111111);  // IODIRB

	ret = i2cMasterSendNI(TWI_ADDR_WRITE, 3, (u08 *)data);
	if (ret) goto out;  // make sure we got an ACK
	// set pull-up
	// - unused  : on  : 1
	// - input   : on  : 1
	// - driving : off : 0
	data[0] = GPPUA;
	data[1] = 0b00000000;  // IODIRA
	data[2] = (0b11111111);  // IODIRB

	ret = i2cMasterSendNI(TWI_ADDR_WRITE, 3, (u08 *)data);
	if (ret) goto out;  // make sure we got an ACK

	// set logical value (doesn't matter on inputs)
	// - unused  : hi-Z : 1
	// - input   : hi-Z : 1
	// - driving : hi-Z : 1
	data[0] = OLATA;
	data[1] = 0b11111111;  // IODIRA
	data[2] = (0b11111111);  // IODIRB

	ret = i2cMasterSendNI(TWI_ADDR_WRITE, 3, (u08 *)data);

out:
	return ret;
}

void initMatrix(void){
    
    delegateInitMatrixDevice();

    debounceMAX = 7;
    debounce = 10;

#ifdef GHOST_KEY_PREVENTION 
    ghostFilterMatrixPointer = currentMatrix;
#endif

    // initialize i2c function library
	i2cInit();
	i2cSetBitrate(400);

//	mcp23018_init();
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
    _isReleaseAllPrev = true;
    for(row=0;row<ROWS;row++) {
        if(prevMatrix[row] > 0){
            _isReleaseAllPrev = false;
            break;
        }
    }
}

bool isReleaseAll(void){    
    return _isReleaseAll;
}
bool isReleaseAllPrev(void){
    return _isReleaseAllPrev;
}
bool isFnPressed(void){
    return _isFnPressed;
}

// function that determine keymap
// 0 = normal, 1 = fn, 2 = beyond_fn
uint8_t getLayer(void) {
    uint8_t col, row, keyidx, cur, gLayer;

    /*

    게으른 FN이 작동되는 상황 정리;
    - 첫키로 FN이 눌려야 한다. 이미 다른 키가 눌려있다면 작동 안 함; <- 불편할 수 있으므로
        기본 레이어일 경우에는 다른 키들이 눌려져 있어도 FN이 작동하도록 한다.
        : 
    - 작동이 된 후에는 모든 키가 release 되는 순간까지 layer를 유지 시킨다.
    (즉, 모든 키가 release 되고 1프레임 후에 작동 해제 되어야한다. 
    ps2의 경우 제일 마지막 키의 release값을 처리해야하기 때문에.)
    */

    if(_currentLazyLayer > 0) return _currentLazyLayer;

    gLayer= 0;
    // fn이 가장 우선, 다음 fn2
    uint8_t *gMatrix = getCurrentMatrix();
    for(row=0;row<ROWS;row++){  
        if(gMatrix[row] == 0) continue;
        for(col=0;col<COLUMNS;col++){   

            cur  = gMatrix[row] & BV(col);

            if(cur){
                keyidx = getCurrentKeyindex(_fnScanLayer, row, col);
                keyidx = getDualActionKeyWhenCompound(keyidx);  // fn 키는 무조건 다운 액션을 적용;
                
#ifdef DEBUG_QUICK_BOOTLOADER
    // for test
    if(col == 0 && row == 0){
        wdt_enable(WDTO_15MS);
        while(1);
    }
#endif              
                // DEBUG_PRINT(("col= %d, row= %d keymap\n", col, row));
                if(keyidx == KEY_FN){
                    gLayer = LAYER_FN;
                }else if(keyidx == KEY_FN2){
                    gLayer = LAYER_FN2;
                }else if(keyidx == KEY_FN3){
                    gLayer = LAYER_FN3;     
                }else if(keyidx == KEY_NOR){
                    if(_fnScanLayer == LAYER_FN2){
                        // _fnScanLayer은 2를 유지하면서 스캔할 레이어는 0으로 반환;
                        _isFnPressed = true;
                        return LAYER_NORMAL;
                    }
                }
                if(gLayer > 0){                 
                    // _fnScanLayer은 0을 유지하면서 스캔할 레이어는 1로 반환;
                    if(isLazyFn()){
                        if(isReleaseAllPrev() || _isFnPressed == false){
                            _currentLazyLayer = gLayer;
                        }else{
                            return _fnScanLayer;
                        }
                    }
                    _isFnPressed = true;
                    return gLayer;
                }
            }
        }
    }
    
    if(isBeyondFN()) {
        _fnScanLayer = isBeyondFN();
    }else{      
        _fnScanLayer = LAYER_NORMAL;
    }
    _isFnPressed = false;
    return _fnScanLayer;
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
    uint8_t col, row;
    uint8_t prev, cur;
    
    uint8_t isModified = 0;

    for(col=0;col<COLUMNS;col++)
    {
        // Col -> set only one port as input and all others as output low
        delegateSetCellStatus(col);
        
        // scan each rows
        for(row=0;row<MCP_ROWS_START;row++)
        {
            cur = delegateGetCellStatus(row);

            prev = currentMatrix[row] & BV(col);

            if(!(prev && cur) && !(!prev && !cur)) {
                if(cur)
                    currentMatrix[row] |= BV(col);
                else
                    currentMatrix[row] &= ~BV(col);

                isModified = 1;
            }
        }

    }

    // i2c
    uint8_t ret, data;
    uint8_t txdata[3];

	// initialize things, just to make sure
	// - it's not appreciably faster to skip this, and it takes care of the
	//   case when the i/o expander isn't plugged in during the first
	//   init()
	ret = mcp23018_init();

	// if there was an error
	if (ret) {
		goto next;
	}


	// --------------------------------------------------------------------
	// update our part of the matrix
	for (col=0; col < COLUMNS; ++col) {
		// set active column low  : 0
		// set other columns hi-Z : 1
		txdata[0] = (GPIOA);
		txdata[1] = ( 0xFF & ~(1<<col) );
		i2cMasterSendNI(TWI_ADDR_WRITE, 2, (u08 *)txdata);

		// read row data
		i2cMasterReceiveNI(TWI_ADDR_WRITE, 1, (u08 *)&data);

		// update matrix
		for (row=MCP_ROWS_START; row < ROWS; ++row) {

			// data: 현재 col의 row 데이터, 0bit가 on
			cur = ((~data)>>(row-MCP_ROWS_START)) & 0x01; //(~data) & BV(row);

			prev = currentMatrix[row] & BV(col);

			if(!(prev && cur) && !(!prev && !cur)) {
				if(cur)
					currentMatrix[row] |= BV(col);
				else
					currentMatrix[row] &= ~BV(col);

				isModified = 1;
			}
		}

	}

	// set all columns hi-Z : 1
	txdata[0] = (GPIOA);
	txdata[1] = ( 0xFF );
	i2cMasterSendNI(TWI_ADDR_WRITE, 2, (u08 *)txdata);

next:

    if(isModified){
        debounce=0;
    }else if(debounce<100){ // to prevent going over limit of int
        // 키 입력에 변화가 없다면 99에서 멈춰서 0을 계속 반환하게 된다. 때문에, 키 변화없을때는 키코드 갱신없음;
        debounce++;
    }

    if(debounce != debounceMAX){
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

    // DEBUG_PRINT(("GHOST_KEY_PREVENTION  p : %d, prevMatrix : %d, currentMatrix : %d \n", ghostFilterMatrixPointer, prevMatrix, currentMatrix));
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

void setPrevMatrix(void){
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
    setReleaseAll();

    if(isReleaseAll()){
        _currentLazyLayer = 0;
    }
}

