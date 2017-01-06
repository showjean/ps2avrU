/* Copyright Jamie Honan, 2001.  Distributed under the GPL.
   This program comes with ABSOLUTELY NO WARRANTY.
   See the file COPYING for license details.
   */

#define KEYBD_EXTERN

#include "ps2main.h"

#include "global.h"
#include "timerinclude.h"
// #include "print.h"
#include "keymap.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#include "keysta.h"

#include "eeprominfo.h"
#include "hardwareinfo.h"
#include "keymatrix.h"
#include "ledrender.h"
#include "quickmacro.h"
#include "keyindex.h"
#include "macrobuffer.h"
#include "enterframe.h"
#include "keydownbuffer.h"
#include "fncontrol.h"
#include "dualaction.h"
#include "keyscan.h"
#include "sleep.h"
#include "oddebug.h"

// Output buffer - circular queue
#define QUEUE_SIZE 200
static uint8_t QUEUE[QUEUE_SIZE];
static int rear=0, front=0;

static uint8_t lastMAKE_keyidx;
static uint8_t lastMAKE[10];
static uint8_t lastMAKE_SIZE=0;
static uint8_t lastMAKE_IDX=0;
static long loopCnt;

static uint8_t TYPEMATIC_DELAY=2;
static long TYPEMATIC_REPEAT=5;

unsigned char txScanCode = 0; // scancode being urrently transmitted
unsigned char m_state;
unsigned char lastSent;
unsigned char lastState;

// key information for each keys
static uint8_t KFLA[NUM_KEY];

// initialize speed port
static unsigned char rxed;

static int temp_a, temp_b;
static int i, j;
static int keyval=0;

// waiting RX, for led indicate key like capslock, because, fix bug of fast down/up like macro/dualaction.
static bool _isWaitingRx = false;
static void setWaitingRx(bool xIsWait);
static bool isWaitingRx(void);
static bool isAlreadyPushedWaitingRx = false;

uint8_t ps2_repeat_speed = PS2_REPEAT_SPEED_NONE;

/* ------------------------------------------------------------------------- */
/* ----------------------------- PS/2 interface ----------------------------- */
/* ------------------------------------------------------------------------- */
// Queue operation -> push, pop
static void push(uint8_t item) {
    static uint8_t record=0;

    if(item==START_MAKE) {
        lastMAKE_SIZE=0;
        record=1;
        return;
    }
    if(item==END_MAKE) {
        record=0;
        return;
    }
    if(item==NO_REPEAT) {
        lastMAKE_SIZE=0;
        record=0;
        return;
    }

    if(record)
        lastMAKE[lastMAKE_SIZE++] = item;

    rear = (rear+1)%QUEUE_SIZE;
    if(front==rear) {
        rear = (rear!=0) ? (rear-1):(QUEUE_SIZE-1);
        return;
    }
    QUEUE[rear] = item;
}

static uint8_t pop(void) {
    if(front==rear) {
        return 0;
    }
    front = (front+1)%QUEUE_SIZE;

    return QUEUE[front];
}

static uint8_t isEmpty(void) {
    if(front==rear)
        return 1;
    else
        return 0;
}

static void clear(void) {
    rear = front = 0;
    lastMAKE_SIZE=0;
    lastMAKE_IDX=0;
    loopCnt=0;
}

static void tx_state(unsigned char x, unsigned char newstate)
{
    //DEBUG_PRINT(("[%02x] [%d] tx_state\n", x, newstate));
    if(x != 0xFE)
        lastSent=x;
    kbd_set_tx(x);
    m_state = newstate;

}

static void keymap_init(void) 
{
    int i, keyidx;

    // set zero for every flags
    for(i=0;i<NUM_KEY;i++)
        KFLA[i]=0;
    
    // set flags
    for(i=0;(keyidx=pgm_read_byte(&keycode_set2_special[i]))!=KEY_NONE;i++)
        KFLA[keyidx] |= KFLA_SPECIAL;
    for(i=0;(keyidx=pgm_read_byte(&keycode_set2_makeonly[i]))!=KEY_NONE;i++)
        KFLA[keyidx] |= KFLA_MAKEONLY;
    for(i=0;(keyidx=pgm_read_byte(&keycode_set2_make_break[i]))!=KEY_NONE;i++)
        KFLA[keyidx] |= KFLA_MAKE_BREAK;
    for(i=0;(keyidx=pgm_read_byte(&keycode_set2_extend[i]))!=KEY_NONE;i++)
        KFLA[keyidx] |= KFLA_EXTEND;
    for(i=0;(keyidx=pgm_read_byte(&keycode_set2_proc_shift[i]))!=KEY_NONE;i++)
        KFLA[keyidx] |= KFLA_PROC_SHIFT;
    for(i=0;(keyidx=pgm_read_byte(&keycode_set2_wait_until_rx[i]))!=KEY_NONE;i++)
        KFLA[keyidx] |= KFLA_WAIT_UNTIL_RX;
}

void setWaitingRx(bool xIsWait){
	if(xIsWait) {
		keyval = SPLIT;
	}else{
		isAlreadyPushedWaitingRx = false;
	}

    _isWaitingRx = xIsWait;
}
bool isWaitingRx(void){
    return _isWaitingRx;
}

/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  PS/2 ----------------------------- */
/* ------------------------------------------------------------------------- */
/*static uint8_t pushKeyCodeDummy(uint8_t keyidx, bool isDown){
    return 0;
}*/
static uint8_t pushKeyCode(uint8_t keyidx, bool isDown)
{
    if(keyidx == KEY_NONE) return 0;

    if(keyidx >= KEY_MAX) return 0;

    DBG1(0x20, (uchar *)&keyidx, 2);

    // if prev and current state are different,
    uint8_t keyVal = pgm_read_byte(&keycode_set2[keyidx]);

    if(isDown) {        // make code
        lastMAKE_keyidx = keyidx;
        loopCnt=0;
        m_state = STA_NORMAL;

        if(KFLA[keyidx]&KFLA_SPECIAL) {
            switch(keyidx) {
                case KEY_PRNSCR:
                    push(START_MAKE);
                    push(0xE0);
                    push(0x12);
                    push(0xE0);
                    push(0x7C);
                    push(END_MAKE);
                    push(SPLIT); // SPLIT is for make sure all key codes are transmitted before disturbed by RX
                    break;
                case KEY_PAUSE:
                    push(NO_REPEAT);
                    push(0xE1);
                    push(0x14);
                    push(0x77);
                    push(0xE1);
                    push(0xF0);
                    push(0x14);
                    push(0xF0);
                    push(0x77);
                    push(SPLIT);
                    break;
            }
        }else if(KFLA[keyidx]&KFLA_MAKEONLY) {
            push(keyVal);
            push(SPLIT);
        }else{
            push(START_MAKE);
            if(KFLA[keyidx]&KFLA_EXTEND) 
                push(0xE0);
            push(keyVal);

            push(END_MAKE);
            push(SPLIT);

        }


        // 2 연속으로 waiting rx 키가 입력 되면 오류로 키보드가 멈춘다. 이를 방지 하기 위해서 isAlreadyPushedWaitingRx를 설정;
        if((KFLA[keyidx] & KFLA_WAIT_UNTIL_RX) && isAlreadyPushedWaitingRx == false){
        	isAlreadyPushedWaitingRx = true;
            if(keyidx == KEY_SCRLCK){
            	// ctrl + scroll lock 키는 LED 반응을 하지 않으니 설정안함
            	// ctrl + alt + s/l 등 다른 조합키와 섞이면 반응함
            	uint8_t gModi = getModifierDownBuffer();
				if(!(gModi == 0x01) || !(gModi == 0x10) || !(gModi == 0x11)){
	            	push(WAIT_RX);
				}
            }else{
            	push(WAIT_RX);
            }
        }
    }
    else            // break code - key realeased
    {
        if(lastMAKE_keyidx == keyidx)       // repeat is resetted only if last make key is released
            lastMAKE_SIZE=0;

        if(KFLA[keyidx]&KFLA_SPECIAL) {
            switch(keyidx) {
                case KEY_PRNSCR:
                    push(0xE0);
                    push(0xF0);
                    push(0x7C);
                    push(0xE0);
                    push(0xF0);
                    push(0x12);
                    push(SPLIT);
                    break;
            }
        }else if(KFLA[keyidx]&KFLA_MAKEONLY) {
        }else{
            if(KFLA[keyidx]&KFLA_EXTEND)
                push(0xE0);

            push(0xF0);
            push(keyVal);
            push(SPLIT);

            // DBG1(0x21, (uchar *)&keyidx, 1); 
        }
    }
    return 1;
}

static void scanKeyPs2WithMacro(void){

    scanKeyWithMacro();
}

static void initPs2(void)
{
    if(interfaceReady == false){
        interfaceReady = true;

        initAfterInterfaceMount();

    }
} 

bool hasUpdatePs2(void){
	return !isEmpty();
}

static interface_update_t updatePs2 = {
		hasUpdatePs2         // hasUpdate
};

static keyscan_driver_t driverKeyScanPs2 = {
//    pushKeyCode,        // pushKeyCode
    pushKeyCode         // pushKeyCodeWhenChange
};


/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  Main  ----------------------------- */
/* ------------------------------------------------------------------------- */

static void processRxPs2(void){

    // check that every key code for single keys are transmitted        
    if (kbd_flags & FLA_RX_BAD) {     // pokud je nastaveny flag spatneho prijmu, zrus ho 
        // pokud flag is set back income withdrawn
        cli();
        kbd_flags &= ~FLA_RX_BAD;
        sei();
    }

    if ( isReadyForRx() && (keyval==SPLIT || isEmpty())) {     // pokud nastaveny flag prijmu bytu, vezmi ho a zanalyzuj
        // pokud law, the flag setting apart, take it and zanalyzuj
        rxed = kbd_get_rx_char();  

        setWaitingRx(false);     

        switch(m_state) {
            default:
                switch(rxed) {
                    case 0xEE: /* echo */
                        // DEBUG_PRINT((" echo \n"));
                        DBG1(0xd0, (uchar *)&rxed, 1); 
                        tx_state(0xEE, m_state);
                        return;
                    case 0xF2: /* read id */
                        // DEBUG_PRINT((" read id \n"));
                        DBG1(0xd1, (uchar *)&rxed, 1); 
                        tx_state(0xFA, STA_WAIT_ID);
                        return;
                    case 0xFF: /* reset */
                        // DEBUG_PRINT((" reset \n"));
                        DBG1(0xd2, (uchar *)&rxed, 1); 
                        tx_state(0xFA, STA_WAIT_RESET);
                        return;
                    case 0xFE: /* resend */
                        // DEBUG_PRINT((" resend \n"));
                        DBG1(0xd3, (uchar *)&rxed, 1); 
                        tx_state(lastSent, m_state);
                        return;
                    case 0xF0: /* scan code set */
                        // DEBUG_PRINT((" scan code set \n"));
                        DBG1(0xd4, (uchar *)&rxed, 1);      // 2
                        tx_state(0xFA, STA_WAIT_SCAN_SET);
                        return;
                    case 0xED: /* led indicators */ 
                        // DEBUG_PRINT((" led indicators \n")); 
                        DBG1(0xd5, (uchar *)&rxed, 1);      // 4
                        tx_state(0xFA, STA_WAIT_LEDS);
                        return;
                    case 0xF3:
                        // DEBUG_PRINT((" STA_WAIT_AUTOREP \n"));
                        DBG1(0xd6, (uchar *)&rxed, 1);      // 3
                        tx_state(0xFA, STA_WAIT_AUTOREP);
                        return;
                    case 0xF4:      // enable
                        // DEBUG_PRINT((" enable \n"));
                        DBG1(0xd7, (uchar *)&rxed, 1);      // 5
                        tx_state(0xFA, STA_NORMAL);

                        initPs2();

                        return;
                    case 0xF5:      // disable
                        // DEBUG_PRINT((" disable \n"));
                        DBG1(0xd8, (uchar *)&rxed, 1); 
                        tx_state(0xFA, STA_NORMAL);
                        return;
                    case 0xF6:      // Set Default
                        // DEBUG_PRINT((" Set Default \n"));
                        DBG1(0xd9, (uchar *)&rxed, 1);  // 1
                        TYPEMATIC_DELAY=1;
                        TYPEMATIC_REPEAT=5;
                        clear();
                        tx_state(0xFA, STA_NORMAL);
                        return;
                    default:
                        // DEBUG_PRINT((" default \n"));
                        DBG1(0xda, (uchar *)&rxed, 1); 
                        tx_state(0xFA, STA_NORMAL);
                        break;
                }
                return;
            case STA_RXCHAR:
//                DEBUG_PRINT((" STA_RXCHAR \n"));
                if (rxed == 0xF5)
                    tx_state(0xFA, STA_NORMAL);
                else {
                    tx_state(0xFA, STA_RXCHAR);
                }
                return;

            case STA_WAIT_SCAN_SET:
//                DEBUG_PRINT((" STA_WAIT_SCAN_SET \n"));

                clear();
                tx_state(0xFA, rxed == 0 ? STA_WAIT_SCAN_REPLY : STA_NORMAL);
                return;
            case STA_WAIT_AUTOREP:
//                DEBUG_PRINT((" STA_WAIT_AUTOREP STA_WAIT_AUTOREP \n"));
                TYPEMATIC_DELAY = (rxed&0b01100000)/0b00100000;

                temp_a = (rxed&0b00000111);
                temp_b = (rxed&0b00011000)/(0b000001000);

                j=1;
                for(i=0;i<temp_b;i++) {
                    j = j*2;
                }

                TYPEMATIC_REPEAT = temp_a*j;

                tx_state(0xFA, STA_NORMAL);
                
                return;
            case STA_WAIT_LEDS:
            	DBG1(0xdb, (uchar *)&rxed, 1);

                initPs2();
                
                uint8_t ledstate = 0;

                // scroll lock
                if(rxed&0x01)
                    ledstate |= LED_STATE_SCROLL;                   

                // num lock
                if(rxed&0x02)
                    ledstate |= LED_STATE_NUM;
                                    
                // capslock
                if(rxed&0x04)
                    ledstate |= LED_STATE_CAPS;
                
                setLEDState(ledstate);
                setLEDIndicate();

                tx_state(0xFA, STA_NORMAL);
                return;
        }
    }
}

static void processTxPs2(void){
    if (isReadyForTx() && !isWaitingRx()) {   // pokud flag odesilani ok -> if the flag sent ok

        switch(m_state) {
            case STA_NORMAL:

                // if error during send
                if(isEmpty()){
                    scanKeyPs2WithMacro();
                }

                keyval = pop();

                if(keyval == WAIT_RX) {
                    setWaitingRx(true);
                    return;
                }

                if(keyval==SPLIT)
                    return;

                if(keyval) {
                    tx_state(keyval, STA_NORMAL);

                    loopCnt=0;

                }else if(lastMAKE_SIZE>0) {     // means key is still pressed
                    loopCnt++;

                    // if key is pressed until typmatic_delay, goes to repeat the last key
                    if(loopCnt >= TYPEMATIC_DELAY*150+230) {
                        loopCnt=0;
                        lastMAKE_IDX=0;
                        m_state = STA_REPEAT;                           
                    }
                }

                // ps2avrU loop, must be scan matrix;
                enterFrame();

                break;
            // typematic : repeat last key
            case STA_REPEAT:
                
                if(lastMAKE_IDX==0) {   // key state can be escaped only if whole key scancode is transmitted
                    scanKeyPs2WithMacro();
                }

                if(lastMAKE_SIZE==0 || !isEmpty()) {    // key is released. go to normal
                    m_state=STA_NORMAL;
                    loopCnt=0;
                    break;
                }

                // if release key is pushed, send them.
                if(loopCnt==1 || lastMAKE_IDX!=0) {
                    tx_state(lastMAKE[lastMAKE_IDX++], STA_REPEAT);
                    lastMAKE_IDX %= lastMAKE_SIZE;
                }
                
                loopCnt++;
                loopCnt %= (3+TYPEMATIC_REPEAT*10) * ps2_repeat_speed;

                // ps2avrU loop, must be scan matrix;
                enterFrame();
                
                break;
            case STA_WAIT_SCAN_REPLY:
                tx_state(0x02, STA_NORMAL);
                break;
            case STA_WAIT_ID:
                tx_state(0xAB, STA_WAIT_ID1);
                break;
            case STA_WAIT_ID1:
                tx_state(0x83, STA_NORMAL);
                break;

            case STA_WAIT_RESET:
                clear();

                tx_state(0xAA, STA_NORMAL);
                break;
        }
    }
}

void ps2_main(void){

    m_state = STA_WAIT_RESET;
    kbd_init();

    keymap_init();
    clear();

    //set key repeat speed;
    uint8_t gSpeed = eeprom_read_byte((uint8_t *)EEPROM_PS2_REPEAT_SPEED);
    if(gSpeed == 0xFF) gSpeed = 1;
    if(ps2_repeat_speed == PS2_REPEAT_SPEED_NONE){
    	ps2_repeat_speed = gSpeed;
    }else if(ps2_repeat_speed != gSpeed){
        eeprom_update_byte((uint8_t *)EEPROM_PS2_REPEAT_SPEED, ps2_repeat_speed);
    }

    // init
    setKeyScanDriver(&driverKeyScanPs2);
    setUpdateDriver(&updatePs2);
    clearMatrix();

    wakeUp();

    sei();

    for(;;){

        // 카운트 이내에 신호가 잡히지 않으면 이동;
        // 특별한 경우에만 발생하는 현상이다.
       /* if(INTERFACE == INTERFACE_PS2 && interfaceReady == false && interfaceCount++ > 1000){
            // move to usb
            INTERFACE = INTERFACE_USB;
            DBG1(0x99, 0, 0);
            break;
        }*/

        processRxPs2();
        processTxPs2();

    }
}
