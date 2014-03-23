/**
 * \file source/keymain.c
 * \brief Main functions for ps2avrU
 * \author showjean <showjean@hanmail.net>
 * \version v 1.0 2013-10-21 11:51
 *
 * License: GNU GPL v2
 */

#include "keymain.h"

#include "timerinclude.h"
#include "global.h"

#include "print.h"
#include "ps2avru_util.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/eeprom.h>

#include <util/delay.h>
#include <string.h>

#include "hardwareinfo.h"
#include "hardwareconfig.h"
#include "keymap.h" 
#include "keymapper.h"
#include "keyindex.h"
#include "quickswap.h"
#include "lazyfn.h"
#include "smartkey.h"
#include "ledrender.h"
#include "keyindex.h"
#include "ps2main.h"
#include "usbmain.h"
#include "bootmapper.h"
#include "custommacro.h"
#include "fncontrol.h"

#include "oddebug.h"

#ifdef DEBUG_QUICK_BOOTLOADER
#warning "======================================================="
#warning "Never compile production devices with QUICK_BOOTLOADER enabled"
#warning "======================================================="
#endif

/* ------------------------------------------------------------------------- */
/* -----------------------------    variable  global ----------------------------- */
/* ------------------------------------------------------------------------- */
int interfaceCount = 0;
bool interfaceReady = false;
uint8_t INTERFACE = 255;
/* ------------------------------------------------------------------------- */

void setUsbOn(void) {   
    // TR에 전압을 연결하기 위해서 D5를 설정; input, pull up
    // 이러면 제너다이오드가 GND와 연결되어 USB로 작동가능;
#ifndef DISABLED_TR_SWITCH  
    cbi(DIODE_DDR, DIODE_PIN); //DIODE_DDR  &= ~BV(DIODE_PIN);
    sbi(DIODE_PORT, DIODE_PIN); //DIODE_PORT |= BV(DIODE_PIN);
#endif
}
void setUsbOff(void) {      
    // PS/2로 사용하기위해서 output low
    // 이러면 제너다이오드가 GND와 연결이 차단된다.
#ifndef DISABLED_TR_SWITCH  
    sbi(DIODE_DDR, DIODE_PIN); //DIODE_DDR  |= (1 << DIODE_PIN);
    cbi(DIODE_PORT, DIODE_PIN); //DIODE_PORT &= ~(1 << DIODE_PIN);
#endif
}

static void clearInterface(void){
    cli();
    clearLEDInited();
    interfaceCount = 0;
    interfaceReady = false;
}

static void clearTimers(void) {
    // timer들을 리셋하지않으면 USB 인식이 불안정하다.

    u08 intNum;
    // detach all user functions from interrupts
    for(intNum=0; intNum<TIMER_NUM_INTERRUPTS; intNum++)
        timerDetach(intNum);

    // timer0, timer1 reset;
    cbi(TIMSK, TOIE0);  // disable TCNT0 overflow
    cbi(TIMSK, TOIE1);  // disable TCNT1 overflow
    // timer2 reset
    cbi(TIMSK, TOIE2);  // disable TCNT2 overflow
}

/**
 * Initialize hardware. Configure ports as inputs and outputs, set USB reset
 * condition, start timer and blink LEDs.
 */
static void initHardware(bool xIsUSB) {

    initMatrix();

    if(xIsUSB){
        clearTimers();
        setUsbOn();
    }else{
        setUsbOff();
    }
}

int setDelay(int xDelay){
    if(INTERFACE == INTERFACE_PS2 || INTERFACE == INTERFACE_PS2_USER){      
        return xDelay >> 1; // ps2의 경우 USB보다 대기 시간이 길어서 반으로 줄여줌;
    }
    return xDelay;
}
void initAfterInterfaceMount(void){
    initFullLEDState();
    initCustomMacro();
}

static void initPreparing(void){
    // init sw
    initKeymapper();
    initQuickSwap();
    initLazyFn();
    initSmartKey();
    initBeyondFn();

    // init
    initLED();
    clearInterface();
}

int main(void) {

    enable_printf();
    odDebugInit();

    DBG1(0x00, 0, 0);

    INTERFACE = 255;
    uint8_t ckeckNeedInterface = 0;

    // for interface select
    // initHardware(0); //여기서 이 함수를 사용하면 ps/2인식 오류를 발생시킴;

    initMatrix();
    clearMatrix();  // 전체를 통틀어 여기에서 1번만 실행시킨다.

    _delay_us(5);

    uint8_t escapeCounter = 0;
    while(getLiveMatrix() == 0 && ++escapeCounter < 20){
        // waiting during clear debounce
    }

    uint8_t row, col, cur, keyidx;  
    uint8_t *gMatrix = getCurrentMatrix();

    // debounce cleared => compare last matrix and current matrix
    for(row=0;row<ROWS;row++)
    {   
        if(gMatrix[row] == 0) continue;
        for(col=0;col<COLUMNS;col++)
        {
            cur  = gMatrix[row] & BV(col);
            // DEBUG_PRINT(("keyidx : %d, row: %d, matrix : %s \n", keyidx, row, currentMatrix[row]));  
            if( cur ) {
                keyidx = getKeyIndex(0, row, col);
#ifdef ENABLE_BOOTMAPPER
                if(getBootmapperStatus(col, row) || keyidx == KEY_TAB){ // bootmapper start
                    setToBootMapper();
                    continue;
                }
#endif
                if(keyidx == KEY_M) {
                    readyKeyMappingOnBoot();
                }else if(keyidx == KEY_U) {
                    ckeckNeedInterface |= (1 << 0);
                }else if(keyidx == KEY_P) {
                    ckeckNeedInterface |= (1 << 1);
                }
            }

        }
        
    }
    setPrevMatrix();
    setCurrentMatrixAfter();


#ifdef INTERFACE_ONLY_USB
    INTERFACE = INTERFACE_USB_USER;
#else
    
    // DEBUG_PRINT(("ckeckNeedInterface %02x \n", ckeckNeedInterface));
    if(ckeckNeedInterface > 0){
        INTERFACE = ckeckNeedInterface + 1; 
    }  

    DEBUG_PRINT(("user selected INTERFACE %02x \n", INTERFACE));

    if(INTERFACE == INTERFACE_CLEAR){
        eeprom_write_byte((uint8_t *)EEPROM_INTERFACE, 255);  // eeprom clear; // 1바이트 12번지 쓰기
        // DEBUG_PRINT(("--------------------------eeprom   write   INTERFACE %02x \n", 255));
    }else if(INTERFACE < INTERFACE_CLEAR){
        eeprom_write_byte((uint8_t *)EEPROM_INTERFACE, INTERFACE);  // 1바이트 12번지 쓰기
        // DEBUG_PRINT(("--------------------------eeprom   write   INTERFACE %02x \n", INTERFACE));
    }else{
        INTERFACE = eeprom_read_byte((uint8_t *)EEPROM_INTERFACE);    // 1바이트 12번지 읽기, 기본값 0xFF ( 255)  
        // DEBUG_PRINT(("eeprom   read   INTERFACE %02x \n", INTERFACE));
    }

    // preparing auto detacting
    P2U_PS2_DDR     &= ~BV(P2U_PS2_CLOCK_PIN);
    P2U_PS2_PORT    |= BV(P2U_PS2_CLOCK_PIN);
    uint8_t debounce = 0;
    uint8_t prevPIN = 0;

    // 인터페이스 선택이 없었다면 자동 인식 실행;
    if(INTERFACE >= INTERFACE_CLEAR){
        for(;;){

            /*
            DDRD    = 0b00010011; 
            PORTD   = 0b11101100; 

            usb  0xd8 = 0b11011000 => 0xc8 = 0b11001000
            ps/2  0xdc = 0b11011100 => 0xcc = 0b11001100
            */
            
            if((P2U_PS2_PINS&(BV(P2U_PS2_CLOCK_PIN))) == 0){
                if((prevPIN&(BV(P2U_PS2_CLOCK_PIN))) == 0){
                    debounce++;
                    if(debounce > 5){
                        // usb
                        INTERFACE = INTERFACE_USB;
                    }
                }else{
                    debounce = 0;
                }
                prevPIN = P2U_PS2_PINS;
            }else if(P2U_PS2_PINS&(BV(P2U_PS2_CLOCK_PIN))){
                if(prevPIN&(BV(P2U_PS2_CLOCK_PIN))){
                    debounce++;
                    if(debounce > 5){
                        // ps2
                        INTERFACE = INTERFACE_PS2;
                    }
                }else{
                    debounce = 0;
                }
                prevPIN = P2U_PS2_PINS;
            }

            if(INTERFACE < 2) break;

        }
    }
#endif

    // DEBUG_PRINT(("INTERFACE %02x \n", INTERFACE));
    
    for(;;){
        if(INTERFACE == INTERFACE_USB || INTERFACE == INTERFACE_USB_USER){
            initPreparing();

            initHardware(true);

            usb_main(); 
        }
        
        if(INTERFACE == INTERFACE_PS2 || INTERFACE == INTERFACE_PS2_USER){
            initPreparing();

            blinkOnce(50);
            initHardware(false);
   
            ps2_main();
        }
    }

    return 1;
}

