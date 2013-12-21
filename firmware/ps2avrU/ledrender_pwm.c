#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/eeprom.h>

#include <util/delay.h>
#include <string.h>

#include "print.h"
#include "ledrender.h"

void setPWM(int xValue){
	timer1PWMBSet(xValue);
}

void stopTimer(void){

}
void startTimer(void){

}


void initFullLEDState(void) {

	if(ledInited) return;
	ledInited = 1;

	// timerInit()는 kbd_init()이후에 실행되면 timer2Init()로 인하여 ps/2키보드가 작동하지 않는다.
	// timerInit();	// timerInit() 전체를 실행하면 kbd_init()시 add한 리스너도 지워저 ps2가 동작을 멈춘다.
	// 때문에 리스너는 지우지 않고 init만 실행;
	// timer0Init();	// timer0은 usb일때 led와 연관이 있고 이에 따라 부팅시 작동되지 않는 현상이 있다.

	timer1Init();
	
	/*#ifdef TCNT2	// support timer2 only if it exists
	timer2Init();	// kbd_init()시 timer2를 사용하므로 이를 초기화하면 안된다.
	#endif*/
	
	timer1PWMInit(8);
	timer1PWMBOn();

	DEBUG_PRINT(("______________________ init LED State_____________\n"));


	_ledBrightnessMax = eeprom_read_byte((uint8_t *)EEPROM_LED_BRIGHTNESS);
	_ledBrightnessMax_saved = _ledBrightnessMax;

	_fullLEDMode = eeprom_read_byte((uint8_t *)EEPROM_LED_MODE);    // 1바이트 11번지 읽기, 기본값 0xFF ( 255)
	if(_fullLEDMode == 255){
		_fullLEDMode = 0;
	}
	_fullLEDMode_saved = _fullLEDMode;

	setFullLEDState();

	if(INTERFACE == INTERFACE_PS2 || INTERFACE == INTERFACE_PS2_USER){		
		beyondFNCountDelay = beyondFNCountDelay >> 1;	// ps2의 경우 USB보다 대기 시간이 길어서 반으로 줄여줌;
	}

}

