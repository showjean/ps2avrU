/**
 * \file source/keymain.c
 * \brief Main functions for ps2avrU
 * \author showjean <showjean@hanmail.net>
 * \version v 1.0 2013-10-21 11:51
 *
 * License: GNU GPL v2
 */

#include "keymain.h"

#include "timer.h"
#include "global.h"

#include "print.h"
#include "udelay.h"
#include "ps2avru_util.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/eeprom.h>

#include <util/delay.h>
#include <string.h>

#include "keymap.h" 
#include "hardwareinfo.h"
#include "keymapper.h"
#include "ledrender.h"
#include "ps2main.h"
#include "usbmain.h"


/* ------------------------------------------------------------------------- */
/* -----------------------------    variable  global ----------------------------- */
/* ------------------------------------------------------------------------- */
int interfaceCount = 0;
uint8_t interfaceReady = 0;
uint8_t INTERFACE = 255;
/* ------------------------------------------------------------------------- */

void setUsbOn(void) {	
	// TR에 전압을 연결하기 위해서 D5를 설정;
	// 이러면 제너다이오드가 GND와 연결되어 USB로 작동가능;
	cbi(DIODE_DDR, DIODE_PIN); //DDRD  &= ~BV(DIODE_PIN);
	sbi(DIODE_PORT, DIODE_PIN); //PORTD |= BV(DIODE_PIN);
}
void setUsbOff(void) {		
	// PS/2로 사용하기위해서 output low
	// 이러면 제너다이오드가 GND와 연결이 차단된다.
	sbi(DIODE_DDR, DIODE_PIN); //DDRD  |= (1 << DIODE_PIN);
    cbi(DIODE_PORT, DIODE_PIN); //PORTD &= ~(1 << DIODE_PIN);
}

void clearInterface(void){
	cli();
	clearLEDInited();
	interfaceCount = 0;
	interfaceReady = 0;
}

void clearTimers(void) {
	// timer들을 리셋하지않으면 USB 인식이 불안정하다.

	u08 intNum;
	// detach all user functions from interrupts
	for(intNum=0; intNum<TIMER_NUM_INTERRUPTS; intNum++)
		timerDetach(intNum);

	// timer0, timer1 reset;
	cbi(TIMSK, TOIE0);	// disable TCNT0 overflow
	// TCNT0 = 0;
	cbi(TIMSK, TOIE1);	// disable TCNT1 overflow
	// TCNT1H = 0;
	// TCNT1L = 0;
	// timer2 reset
	cbi(TIMSK, TOIE2);	// disable TCNT2 overflow
	// TCNT2 = 0; //0xEC;
}

/**
 * Initialize hardware. Configure ports as inputs and outputs, set USB reset
 * condition, start timer and blink LEDs.
 */
static void initHardware(uint8_t xIsUSB) {
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
	// PD6 : reserved
    // PD7 : row17
	

	// signal direction : col -> row

	DDRCOLUMNS 	= 0xFF;	// all outputs for cols
	PORTCOLUMNS	= 0xFF;	// pull-up
	DDRROWS1	= 0x00;	// all inputs for rows
	DDRROWS2	= 0x00;
	PORTROWS1	= 0xFF;	// all rows pull-up.
	PORTROWS2	= 0xFF;	
  
	DDRD        = 0x1F; // 0b00011111 all pins output except row 17, 3.6V TR
	PORTD       = 0xE0; // 0b11100000 pull up row 17, 3.6V TR
    
    if(xIsUSB){
    	// USB Reset by device only required on Watchdog Reset	                        
	    _delay_us_m(11);      // delay >10ms for USB reset
		DDRD        = 0x13; //0b00010011  remove USB reset condition

	    // configure timer 0 for a rate of 12M/(1024 * 256) = 45.78Hz (~22ms)
	    TCCR0 = 5;          // timer 0 prescaler: 1024

		clearTimers();

		// initHardware() 보다 나중에 사용되야 한다.	
		setUsbOn();
 	}else{
 		setUsbOff();
 	}
}

uint8_t getUserSelectedInterface(uint8_t pin, uint8_t pinNumber){
	int counter=0;
	int userInterface;
	userInterface = pin & BV(pinNumber);
	while(counter<=10) {                            // PINA5 should be 0 for 10 cycles
		if(userInterface != (pin & BV(pinNumber)))
			counter=0;
		else
			counter++;

		userInterface = (pin & BV(pinNumber));
	}

	// DEBUG_PRINT(("userInteprface %02x ", userInterface));
	return userInterface;
}

int main(void) {

	enable_printf();

    INTERFACE = 255;
	uint8_t ckeckNeedInterface = 0;


	// for interface select
    // initHardware(0);	//여기서 이 함수를 사용하면 ps/2인식 오류를 발생시킴;
    DDRCOLUMNS 	= 0xFF;	// all outputs for cols
	PORTCOLUMNS	= 0xFF;	// pull-up
	DDRROWS1	= 0x00;	// all inputs for rows
	DDRROWS2	= 0x00;
	PORTROWS1	= 0xFF;	// all rows pull-up.
	PORTROWS2	= 0xFF;	
  
	DDRD        = 0b00000000; // row 17 input (PD7)
	PORTD       = 0b10000000; // row 17 pull up (PD7)
    _delay_us_m(1); 

	uint8_t _countDie = 0;
	while(getLiveMatrix() == 0 && ++_countDie < 30){
		// waiting during clear debounce
	}

	// DEBUG_PRINT(("_countDie : %d \n", _countDie));

	
	// DEBUG_PRINT(("checking.... \n"));
	uint8_t row, col, cur, keyidx;	

	// debounce cleared => compare last matrix and current matrix
	for(col=0;col<8;col++)
	{		
		for(row=0;row<17;row++)
		{
			cur  = currentMatrix[row] & BV(col);
			// DEBUG_PRINT(("keyidx : %d, row: %d, matrix : %s \n", keyidx, row, currentMatrix[row]));	
			if( cur ) {
#ifdef ENABLE_BOOTMAPPER
				if(row == 1 && col == 1){	// bootmapper start
					setToBootMapper();
					continue;
				}
#endif
				keyidx = pgm_read_byte(&keymap_code[0][row][col]); //getCurrentKeycode(0, row, col);
				if(keyidx == KEY_M) {
					DEBUG_PRINT(("...........readyKeyMappingOnBoot \n"));
					readyKeyMappingOnBoot();
				}else if(keyidx == KEY_U) {
					DEBUG_PRINT(("KEY_U \n"));
					ckeckNeedInterface |= (1 << 0);
				}else if(keyidx == KEY_P) {
					DEBUG_PRINT(("KEY_P \n"));
					ckeckNeedInterface |= (1 << 1);
				}
			}

		}
		
	}

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
	DDRD    &= ~BV(2);	//0b00010011;	
	PORTD	|= BV(2);	//0b11101100;
	uint8_t debounce = 0;
	uint8_t prevPIND = 0;

	// 인터페이스 선택이 없었다면 자동 인식 실행;
	if(INTERFACE >= INTERFACE_CLEAR){
		for(;;){

			/*
			DDRD    = 0b00010011; 
			PORTD	= 0b11101100; 

			usb  0xd8 = 0b11011000 => 0xc8 = 0b11001000
			ps/2  0xdc = 0b11011100 => 0xcc = 0b11001100
			*/
			
			if((PIND&(BV(2))) == 0){
				if((prevPIND&(BV(2))) == 0){
					debounce++;
					if(debounce > 5){
						// usb
						INTERFACE = INTERFACE_USB;
					}
				}else{
					debounce = 0;
				}
				prevPIND = PIND;
			}else if((PIND&(BV(2))) == 4){
				if((prevPIND&(BV(2))) == 4){
					debounce++;
					if(debounce > 5){
						// ps2
						INTERFACE = INTERFACE_PS2;
					}
				}else{
					debounce = 0;
				}
				prevPIND = PIND;
			}

			if(INTERFACE < 2) break;

		}
	}

	DEBUG_PRINT(("INTERFACE %02x \n", INTERFACE));
	
	for(;;){
		if(INTERFACE == INTERFACE_USB || INTERFACE == INTERFACE_USB_USER){
			clearInterface();
			initHardware(1);

			usb_main();			
		}
		
		if(INTERFACE == INTERFACE_PS2 || INTERFACE == INTERFACE_PS2_USER){
			turnOnLED(LEDNUM | LEDCAPS);
			_delay_ms(100);
			turnOffLED(LEDNUM | LEDCAPS);

			clearInterface();
			initHardware(0);
   
			ps2_main();
		}
	}

	return 1;
}

