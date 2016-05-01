
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/wdt.h>

#include "main.h"
#include "keymatrix.h"
#include "hardwareinfo.h"
#include "boot.h"

#include "i2c/i2c.h"        // include i2c support


// register addresses (see "mcp23018.md")
#define IODIRA 0x00  // i/o direction register
#define IODIRB 0x01
#define GPPUA  0x0C  // GPIO pull-up resistor register
#define GPPUB  0x0D
#define GPIOA  0x12  // general purpose i/o port register (write modifies OLAT)
#define GPIOB  0x13
#define OLATA  0x14  // output latch register
#define OLATB  0x15

#define TW_READ     1
#define TW_WRITE    0

#define MCP23018_TWI_ADDRESS 0b0100000

// TWI aliases
#define TWI_ADDR_WRITE ( (MCP23018_TWI_ADDRESS<<1) | TW_WRITE )
#define TWI_ADDR_READ  ( (MCP23018_TWI_ADDRESS<<1) | TW_READ  )

#define MCP_ROWS_START  9

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


/* ----------------------- hardware I/O abstraction ------------------------ */
#define PORTCOLUMNS PORTB  ///< port on which we read the state of the columns
#define PINCOLUMNS  PINB   ///< port on which we read the state of the columns
#define DDRCOLUMNS  DDRB   ///< port on which we read the state of the columns
#define PORTROWS1   PORTA  ///< first port connected to the matrix rows
#define PINROWS1    PINA   ///< first port connected to the matrix rows
#define DDRROWS1    DDRA   ///< first port connected to the matrix rows
#define PORTROWS2   PORTC  ///< second port connected to the matrix rows
#define PINROWS2    PINC   ///< second port connected to the matrix rows
#define DDRROWS2    DDRC   ///< second port connected to the matrix rows


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

// _______________________________________________________________ boot
void delegateGotoBootloader(void){
    wdt_enable(WDTO_15MS);
    for(;;);

    //          typedef void (*AppPtr_t)(void) __attribute__ ((noreturn));
    //
    //          AppPtr_t AppStartPtr = (AppPtr_t)(BOOTLOADER_START);
    //          AppStartPtr();
    //          ((void (*)(void))(BOOTLOADER_START))();
}

// _______________________________________________________________ keymain
uint8_t delegateGetBootmapperStatus(uint8_t xCol, uint8_t xRow){
    if(xCol == 1 && xRow == 1){
        return 1;
    }else{
        return 0;
    }
}

// _______________________________________________________________ keymatrix
void delegateInitMatrixDevice(void){
    // initialize matrix ports - cols, rows
    // PB0-PB7 : col0 .. col7
    // PA0-PA7 : row0 .. row7
    // PC7-PC0 : row8 .. row15

    // PD0 : NUM
    // PD1 : CAPS
    // PD2 : D+ / Clock
    // PD3 : D- / Data
    // PD4 : FULL LED
    // PD5 : 3.6V switch TR
    // PD6 : SCRL
    // PD7 : row17


    // signal direction : col -> row

    // pc0, 1 : twi
    DDRCOLUMNS  = 0xFF; // all outputs for cols
    PORTCOLUMNS = 0xFF; // high
    DDRROWS1    = 0x00; // all inputs for rows
    DDRROWS2    = 0x00;
    PORTROWS1   = 0xFF; // all rows pull-up.
    PORTROWS2   = 0b11111100; //0xFF;

    DDRD        &= ~(1<<PIND7); // input row 17
    PORTD       |= (1<<PIND7);// pull up row 17


    // initialize i2c function library
    i2cInit();
    i2cSetBitrate(400);

//  mcp23018_init();
}

void delegateSetCellStatus(uint8_t xCol){
    // Col -> set only one port as input and all others as output low
    DDRCOLUMNS  = BV(xCol); // 해당 col을 출력으로 설정, 나머지 입력
    PORTCOLUMNS = ~BV(xCol);    // 해당 col output low, 나머지 컬럼을 풀업 저항

    /*
    DDR을 1로 설정하면 출력, 0이면 입력
    입력중, PORT가 1이면 풀업(풀업 상태는  high 상태);

    출력 상태의 PORT가 0이면 output low(0v);

    스위치를 on하면 0, off하면 1이 PIN에 저장;
    row는 내부 풀업 저항 상태 이기 때문에 1값이 기본값
    */

    _delay_us(5);
}

uint8_t delegateGetCellStatus(uint8_t xRow){

    if(xRow<8)  {               // for 0..7, PORTA 0 -> 7
        return (~PINROWS1) & BV(xRow);
    }else if(xRow<14) { // for 8..13, PORTC 7 -> 0
        return (~PINROWS2) & BV(15-xRow);
    // }else if(14, 15 : 12c
    }else if(xRow==16) { // for 16
        return (~PIND) & BV(7);
    }
    return 0;

}


void delegateGetLiveMatrix(uint8_t *xCurrentMatrix, uint8_t *xIsModified){
    uint8_t col, row;
    uint8_t prev, cur;


    for(col=0;col<COLUMNS;++col)
    {
        delegateSetCellStatus(col);

        // scan each rows
        for(row=0;row<MCP_ROWS_START;++row)
        {
            cur = delegateGetCellStatus(row);

            prev = xCurrentMatrix[row] & BV(col);

            if(!(prev && cur) && !(!prev && !cur)) {
                if(cur)
                    xCurrentMatrix[row] |= BV(col);
                else
                    xCurrentMatrix[row] &= ~BV(col);

                *xIsModified = 1;
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
        return;
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

            prev = xCurrentMatrix[row] & BV(col);

            if(!(prev && cur) && !(!prev && !cur)) {
                if(cur)
                    xCurrentMatrix[row] |= BV(col);
                else
                    xCurrentMatrix[row] &= ~BV(col);

                *xIsModified = 1;
            }
        }

    }

    // set all columns hi-Z : 1
    txdata[0] = (GPIOA);
    txdata[1] = ( 0xFF );
    i2cMasterSendNI(TWI_ADDR_WRITE, 2, (u08 *)txdata);

}
