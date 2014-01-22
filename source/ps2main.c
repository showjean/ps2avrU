/* Copyright Jamie Honan, 2001.  Distributed under the GPL.
   This program comes with ABSOLUTELY NO WARRANTY.
   See the file COPYING for license details.
   */
   
#ifndef PS2MAIN_C
#define PS2MAIN_C

#define KEYBD_EXTERN

#include "ps2main.h"

#include "global.h"
#include "timer.h"
#include "print.h"
#include "keymap.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#include "keysta.h"

#include "hardwareinfo.h"
#include "keymatrix.h"
#include "ledrender.h"
#include "keymapper.h"
#include "keyindex.h"
#include "macrobuffer.h"
#include "enterframe.h"
#include "keydownbuffer.h"
#include "fncontrol.h"
#include "dualaction.h"
#include "lazyfn.h"
#include "keyscan.h"

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

static uint8_t _prevLayer = 0;

unsigned char txScanCode = 0; // scancode being urrently transmitted
unsigned char m_state;
unsigned char lastSent;
unsigned char lastState;

// key information for each keys
uint8_t KFLA[NUM_KEY];

static int scanKeyPS2(void);

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

	clearPrevMatrix();
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
}

/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  PS/2 ----------------------------- */
/* ------------------------------------------------------------------------- */
static uint8_t pushKeyCodeDummy(uint8_t keyidx, bool isDown){
	return 0;
}
static uint8_t pushKeyCode(uint8_t keyidx, bool isDown)
{
	if(keyidx == KEY_NONE) return 0;

     // 듀얼액션 취소되었을 때는 down 키코드를 적용한다.;
    keyidx = getDualActionDownKeyIndexWhenIsCancel(keyidx);

    if(keyidx >= KEY_MAX) return 0;

	// if prev and current state are different,
	uint8_t keyVal = pgm_read_byte(&keycode_set2[keyidx]);

	if(isDown) {		// make code
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
		}
		else if(KFLA[keyidx]&KFLA_MAKEONLY) {
			push(keyVal);
			push(SPLIT);
		}
		else 
		{
			push(START_MAKE);
			if(KFLA[keyidx]&KFLA_EXTEND) 
				push(0xE0);
			push(keyVal);

			push(END_MAKE);
			push(SPLIT);
		}
	}
	else			// break code - key realeased
	{
		if(lastMAKE_keyidx == keyidx)		// repeat is resetted only if last make key is released
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
		}
		else if(KFLA[keyidx]&KFLA_MAKEONLY) {
		}
		else 
		{
			if(KFLA[keyidx]&KFLA_EXTEND)
				push(0xE0);

			push(0xF0);
			push(keyVal);
			push(SPLIT);
		}
	}
	return 1;
}

static int scanKeyPS2(void) {
	
	// debounce cleared and changed
	if(!setCurrentMatrix()) return 0;

    clearDownBuffer();
	
    uint8_t prevKeyidx;
	uint8_t row, col, prev, cur, keyidx;
	uint8_t gLayer = getLayer();

    uint8_t *gMatrix = getCurrentMatrix();
    uint8_t *gPrevMatrix = getPrevMatrix();
    // ps/2 연결시 FN/FN2/NOR키의 레이어 전환시 같은 위치에 있는 다른 키코드의 키가 눌려지지만 손을 때면 눌려진 상태로 유지되는 버그 패치
	// 레이어가 변경된 경우에는 이전 레이어를 검색하여 달리진 점이 있는지 확인하여 적용;
	if(!isLazyFn() && _prevLayer != gLayer){
		for(col=0;col<COLUMNS;col++)
		{		
			for(row=0;row<ROWS;row++)
			{				
				prevKeyidx = getCurrentKeyindex(_prevLayer, row, col);
				keyidx = getCurrentKeyindex(gLayer, row, col);

	            // 이전 상태에서(press/up) 변화가 있을 경우;
	            /*
	            prev : 1, cur : 1 = prev up, cur down
	            prev : 1, cur : 0 = prev up
	            prev : 0, cur : 1 = cur down
	            prev : 0, cur : 0 = -
	            */	            
				if( prevKeyidx != keyidx ) {
					prev = gPrevMatrix[row] & BV(col);
					cur  = gMatrix[row] & BV(col);

					if(prev){
						putChangedKey(prevKeyidx, false, col, row);
					}
					if(cur){
						putChangedKey(keyidx, true, col, row);
					}
				}

			}
			
		}
	}
	_prevLayer = gLayer;

	uint8_t retval = scanKey(gLayer);

    return retval;
}

static uint8_t hasMacroPs2(void)
{
    return !isEmptyM();
}

static int scanKeyPs2WithMacro(void){

    macro_key_t gKey;
    if(hasMacroPs2()){
    	setMacroProcessEnd(false);
	  
        gKey = popMWithKey();
        if(gKey.mode == MACRO_KEY_DOWN){	// down
        	// DEBUG_PRINT(("macro down : %d \n", gKey.keyindex));
        	pushKeyCode(gKey.keyindex, true);
        	push(NO_REPEAT);	// set no repeat
        }else{	// up
        	// 모디키가 눌려져 있다면 그 상태를 유지;
        	if (gKey.keyindex > KEY_Modifiers && gKey.keyindex < KEY_Modifiers_end) {
                if(getModifierDownBuffer() & modmask[gKey.keyindex - (KEY_Modifiers + 1)]){
                	return 0;
                }
            }
        	pushKeyCode(gKey.keyindex, false);
        	// DEBUG_PRINT(("macro up : %d \n", gKey.keyindex));
        }

	    return 0;	 
	}else{   
    	setMacroProcessEnd(true);
	}

	return scanKeyPS2();
}

static void initPs2(void)
{
	interfaceReady = true;

    initFullLEDState();

    startKeyMappingOnBoot();
} 

static keyscan_driver_t driverKeyScanPs2 = {
    pushKeyCode,
    pushKeyCodeDummy,
    pushKeyCode
};

void initInterfacePs2(void){

    setKeyScanDriver(&driverKeyScanPs2);

}

/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  Main  ----------------------------- */
/* ------------------------------------------------------------------------- */

// initialize speed port
static unsigned char rxed;

static int temp_a, temp_b;
static int i, j;
static int keyval=0;
void processRXPs2(void){
	// check that every key code for single keys are transmitted		
	/*if (kbd_flags & FLA_RX_BAD) {		// pokud je nastaveny flag spatneho prijmu, zrus ho 
		// pokud flag is set back income withdrawn
		cli();
		kbd_flags &= ~FLA_RX_BAD;
		sei();
	}*/
	if ((kbd_flags & FLA_RX_BYTE) && (keyval==SPLIT || isEmpty())) {     // pokud nastaveny flag prijmu bytu, vezmi ho a zanalyzuj
		// pokud law, the flag setting apart, take it and zanalyzuj
		rxed = kbd_get_rx_char();		

		switch(m_state) {
			default:
				switch(rxed) {
					case 0xEE: /* echo */
						DEBUG_PRINT((" echo \n"));
						tx_state(0xEE, m_state);
						return;
					case 0xF2: /* read id */
						DEBUG_PRINT((" read id \n"));
						tx_state(0xFA, STA_WAIT_ID);
						return;
					case 0xFF: /* reset */
						DEBUG_PRINT((" reset \n"));
						tx_state(0xFA, STA_WAIT_RESET);
						return;
					case 0xFE: /* resend */
						DEBUG_PRINT((" resend \n"));
						tx_state(lastSent, m_state);
						return;
					case 0xF0: /* scan code set */
						DEBUG_PRINT((" scan code set \n"));
						tx_state(0xFA, STA_WAIT_SCAN_SET);
						return;
					case 0xED: /* led indicators */	
						DEBUG_PRINT((" led indicators \n"));	
						tx_state(0xFA, STA_WAIT_LEDS);
						return;
					case 0xF3:
						DEBUG_PRINT((" STA_WAIT_AUTOREP \n"));
						tx_state(0xFA, STA_WAIT_AUTOREP);
						return;
					case 0xF4:		// enable
						DEBUG_PRINT((" enable \n"));
						tx_state(0xFA, STA_NORMAL);
						initPs2();
						return;
					case 0xF5:		// disable
						DEBUG_PRINT((" disable \n"));
						tx_state(0xFA, STA_NORMAL);
						return;
					case 0xF6:		// Set Default
						DEBUG_PRINT((" Set Default \n"));
						TYPEMATIC_DELAY=1;
						TYPEMATIC_REPEAT=5;
						clear();
					default:
						DEBUG_PRINT((" default \n"));
						tx_state(0xFA, STA_NORMAL);
						break;
				}
				return;
			case STA_RXCHAR:
				DEBUG_PRINT((" STA_RXCHAR \n"));
				if (rxed == 0xF5)
					tx_state(0xFA, STA_NORMAL);
				else {
					tx_state(0xFA, STA_RXCHAR);
				}
				return;

			case STA_WAIT_SCAN_SET:
				DEBUG_PRINT((" STA_WAIT_SCAN_SET \n"));
				// start point... ps2로 인식 후 처음 이곳을 한 번은 거쳐간다?

				clear();
				tx_state(0xFA, rxed == 0 ? STA_WAIT_SCAN_REPLY : STA_NORMAL);
				return;
			case STA_WAIT_AUTOREP:
				DEBUG_PRINT((" STA_WAIT_AUTOREP STA_WAIT_AUTOREP \n"));
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
				// Reflect LED states to PD0~2
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

void processTXPs2(void){
	if (kbd_flags & FLA_TX_OK) {   // pokud flag odesilani ok -> if the flag sent ok
		switch(m_state) {
			case STA_NORMAL:

				// if error during send
				if(isEmpty()){
					scanKeyPs2WithMacro();
				}

		        // ps2avrU loop, must be scan matrix;
		        enterFrame();

				keyval = pop();
				if(keyval==SPLIT)
					return;

				if(keyval) {
					tx_state(keyval, STA_NORMAL);

					loopCnt=0;

				}else if(lastMAKE_SIZE>0) {		// means key is still pressed
					loopCnt++;

					// if key is pressed until typmatic_delay, goes to repeat the last key
					if(loopCnt >= TYPEMATIC_DELAY*150+230) {
						loopCnt=0;
						lastMAKE_IDX=0;
						m_state = STA_REPEAT;							
					}
				}

				break;
			// typematic : repeat last key
			case STA_REPEAT:
				
				if(lastMAKE_IDX==0)	{	// key state can be escaped only if whole key scancode is transmitted
					scanKeyPs2WithMacro();
				}
		        // ps2avrU loop, must be scan matrix;
		        enterFrame();

				if(lastMAKE_SIZE==0 || !isEmpty()) {	// key is released. go to normal
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
				loopCnt %= (3+TYPEMATIC_REPEAT*10);

				
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

				_delay_ms(300);
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

	initInterfacePs2();

	DEBUG_PRINT(("STARTING PS/2 KEYBOARD\n"));

	sei();
	
	while(1) {

		// 카운트 이내에 신호가 잡히지 않으면 이동;
		// 특별한 경우에만 발생하는 현상이다.
		if(INTERFACE == INTERFACE_PS2 && interfaceReady == false && interfaceCount++ > 2000){			
			// move to usb
			INTERFACE = INTERFACE_USB;
			DEBUG_PRINT((" move to usb \n"));
			break;
		}		

		processRXPs2();
		processTXPs2();

	}


	return;	// 0;
}
#endif