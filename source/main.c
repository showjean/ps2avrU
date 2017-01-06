/**
 * \file source/main.c
 * \brief Main functions for ps2avrU
 * \author showjean <showjean@hanmail.net>
 * \version v 1.0 2013-10-21 11:51
 *
 * License: GNU GPL v2
 */

#include "main.h"

#include "timerinclude.h"
#include "global.h"

//#include "print.h"
#include "ps2avru_util.h"

#include <avr/io.h>
//#include <avr/interrupt.h>
//#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/eeprom.h>

#include <util/delay.h>
#include <string.h>

#include "hardwareinfo.h"
#include "hardwareconfig.h"
#include "keymap.h" 
#include "quickmacro.h"
#include "keyindex.h"
#include "quickswap.h"
//#include "lazyfn.h"
#include "ledrender.h"
#include "keyindex.h"
#ifndef INTERFACE_ONLY_USB
#include "ps2main.h"
#endif
#include "usbmain.h"
#include "bootmapper.h"
#include "custommacro.h"
#include "fncontrol.h"
#include "esctilde.h"
#include "keydownbuffer.h"
#include "numlocklayer.h"

#include "oddebug.h"

#if defined(DEBUG_QUICK_BOOTLOADER)
#warning "======================================================="
#warning "Never compile production devices with QUICK_BOOTLOADER enabled"
#warning "======================================================="
#endif

#if defined(DEBUG_KEYMAPPER_H) || defined(DEBUG_SLEEP_H) || defined(DEBUG_PRINT_H) || defined(DEBUG_QUICKSWAP_H)
#warning "======================================================="
#warning "Never compile production devices with DEBUG OPTIONS enabled"
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
    if(INTERFACE == INTERFACE_PS2){
        return xDelay >> 1; // ps2의 경우 USB보다 대기 시간이 길어서 반으로 줄여줌;
    }
    return xDelay;
}

static interface_update_t *updateDriver;
/**
 * 업데이트(키코드 전송)을 해야 하는지 확인하는 드라이버 세팅
 */
void setUpdateDriver(interface_update_t *driver){
    updateDriver = driver;
}

bool hasUpdate(void){
    return updateDriver->hasUpdate();
}

void initAfterInterfaceMount(void){

    initLED();

    initFullLEDState();
    initCustomMacro();
}

static void initPreparing(void){
    // init sw
    initQuickSwap();

    initBeyondFn();     // 1...
    initNumlockLayer();
    initEscTilde();     // last

    initKeyDownBuffer();
}

int main(void) {

    odDebugInit();

    DBG1(0x00, 0, 0);

    // for interface select
    // initHardware(0); //여기서 이 함수를 사용하면 ps/2인식 오류를 발생시킴;

    initMatrix();
    clearMatrix();

    _delay_us(5);

    uint8_t escapeCounter = 0;
    while(getLiveMatrix() == 0 && ++escapeCounter < 20){
        // waiting during clear debounce
    }

    uint8_t row, col, cur, keyidx;
    uint8_t *gMatrix = getCurrentMatrix();

    // debounce cleared => compare last matrix and current matrix
    for(row=0;row<ROWS;++row)
    {
        if(gMatrix[row] == 0) continue;
        for(col=0;col<COLUMNS;++col)
        {
            cur  = gMatrix[row] & BV(col);
            if( cur ) {
                keyidx = getDefaultKeyindex(0, row, col);
#ifdef ENABLE_BOOTMAPPER
                if(delegateGetBootmapperStatus(col, row) || keyidx == KEY_TAB){ // bootmapper start
                    setToBootMapper(true);
                    continue;
                }
#endif

#ifndef INTERFACE_ONLY_USB
                if(keyidx == KEY_U) {
                    INTERFACE = INTERFACE_USB;
                }else if(keyidx == KEY_P) {
                    INTERFACE = INTERFACE_PS2;
                }else if(keyidx == KEY_1) {
                	ps2_repeat_speed = PS2_REPEAT_SPEED_HIGH;
                }else if(keyidx == KEY_2) {
                	ps2_repeat_speed = PS2_REPEAT_SPEED_MIDD;
                }else if(keyidx == KEY_3) {
                	ps2_repeat_speed = PS2_REPEAT_SPEED_LOW;
                }
#endif
            }

        }

    }
    setCurrentMatrixAfter();


#ifdef INTERFACE_ONLY_USB
    INTERFACE = INTERFACE_USB;
#else

    if(INTERFACE <= INTERFACE_USB){
        eeprom_update_byte((uint8_t *)EEPROM_INTERFACE, INTERFACE);
    }else{
        // 255 이면;
        INTERFACE = eeprom_read_byte((uint8_t *)EEPROM_INTERFACE);    // 1바이트 12번지 읽기, 기본값 0xFF ( 255)
    }

    // usb가 default;
    if(INTERFACE != INTERFACE_PS2){
        INTERFACE = INTERFACE_USB;
    }

#endif

    initPreparing();

#ifndef INTERFACE_ONLY_USB
    if(INTERFACE == INTERFACE_PS2){

        clearInterface();
        blinkOnce(50);
        initHardware(false);
        ps2_main();
    }
    else
#endif
    {
        clearInterface();
        initHardware(true);
        usb_main();

    }

    return 1;
}

