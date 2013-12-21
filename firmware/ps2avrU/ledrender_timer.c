
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

volatile int pwmValue = 0;

void stopTimer(void){  
    TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10)); // No clock source (Timer/Counter stopped).
}

void startTimer(void){
	if(pwmValue == 0) return;
    // 타이머, 카운터1번 컨트롤 분주비
    // TCCR1B |= (1<<CS10); //  clk/1 (From prescaler) 
    // TCCR1B |= (1<<CS11); //  clk/8 (From prescaler) 
    // TCCR1B |= (1<<CS10)|(1<<CS11); // clk/64 (From prescaler)
    TCCR1B |= (1<<CS12); //  clk/256 (From prescaler)     
    // TCCR1B |= (1<<CS10)|(1<<CS12); // clk/1024 (From prescaler)
    
}
static void timerProcessCompareB(void){	// like overflow
	turnOnLED(LEDFULLLED);	
	OCR1A = pwmValue;
}
static void timerProcessCompareA(void){
	turnOffLED(LEDFULLLED);
}
void setPWM(int xValue){
	static int prevPwmValue = 1;

    pwmValue = xValue;
    if(pwmValue == 0 && prevPwmValue != 0){
        turnOffLED(LEDFULLLED); 
        stopTimer();
    }else if(pwmValue != 0 && prevPwmValue == 0){
        startTimer();
    }
    prevPwmValue = pwmValue;
}

void initFullLEDState(void) {

	if(ledInited) return;
	ledInited = 1;

	timerAttach(TIMER1OUTCOMPAREA_INT, timerProcessCompareA); 
	timerAttach(TIMER1OUTCOMPAREB_INT, timerProcessCompareB); 	// like overflow

	/*
	커스텀 pwm을 위해서 timer를 사용할때 interupt 간격을 좁게 하면 USB의 작동이 잘 안된다.
	// 인터럽트 루틴에서 너무 많은 일을 처리하면 메인 루틴의 처리가 늦어서 일어나는 현상으로 보여진다.
	인터럽트 루틴에서는 최소한의 처리만 하고 나머지는 메인 루틴에 넘기자.
	*/

	// CTC,  TOP : ICR1
    TCCR1A &= ~(1<<WGM10);
    TCCR1A &= ~(1<<WGM11);
    TCCR1B |= (1<<WGM12);
    TCCR1B |= (1<<WGM13);
    ICR1 = PWM_MAX;
    //output compare value B
    OCR1A = 0;
    OCR1B = ICR1;
    // Normal port operation, OC1A/OC1B disconnected.
    TCCR1A &= ~(1<<COM1A1);
    TCCR1A &= ~(1<<COM1A0);
    TCCR1A &= ~(1<<COM1B1);
    TCCR1A &= ~(1<<COM1B0);

    TCNT1 = 0; // 타이머, 카운터1번 카운팅 레지스터 초기화

	TIMSK |= (1<<OCIE1A);
    TIMSK |= (1<<OCIE1B);

    startTimer();

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
