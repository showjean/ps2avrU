
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "global.h"
#include "common_inc.h"

#include "custommacro.h"
#include "hardwareinfo.h"
#include "macrobuffer.h"
#include "timerinclude.h"
#include "ledrender.h"
#include "oddebug.h"

// #define TICKS_PER_SEC                    1000             // (1/1000)초
#define CUSTOMMACRO_TIMER_COUNT_START   6   //256 - (F_CPU / TICKS_PER_SEC / 64)

#define COUNT_FOR_100_MS                75  //F_CPU / 10 / (256-CUSTOMMACRO_TIMER_COUNT_START) / 64
// 12000000 / 10 / 250 / 64
// 16000000 / 10 / 250 / 64

static uint8_t _currentMacroIndex = 255;
static bool _isActiveCustomMacro = false;
static int _customMacroDelay;
static int macroCounter;
volatile int _countForMs;

static void pushNextKeyIndex(void);
static void timerActionCustomMacro(void);
static void startTimerCustomMacro(void);
static void stopTimerCustomMacro(void);

void initCustomMacro(void){
    timer2Init();
    timerAttach(TIMER2OVERFLOW_INT, timerActionCustomMacro);
    stopTimerCustomMacro();
}

static void timerActionCustomMacro(void){
    outb(TCNT2, CUSTOMMACRO_TIMER_COUNT_START); /* value counts up from this to zero */

    ++_countForMs;
}

static void startTimerCustomMacro(void){
    // timer2SetPrescaler(TIMERRTC_CLK_DIV64);  /* value counts up from this to zero */

    // atmega32 타이머, 카운터2번 컨트롤 분주비
    // atmega128은 다름.
    // TCCR2 |= (1<<CS20); //  clk/1 (From prescaler) 
    // TCCR2 |= (1<<CS21); //  clk/8 (From prescaler) 
    TCCR2 |= (1<<CS21)|(1<<CS20); // clk/32 (From prescaler)
    // TCCR2 |= (1<<CS22); // clk/64 (From prescaler)
    // TCCR2 |= (1<<CS122)|(1<<CS20); //  clk/128 (From prescaler) 
    // TCCR2 |= (1<<CS122)|(1<<CS21); //  clk/256 (From prescaler)     
    // TCCR2 |= (1<<CS22)|(1<<CS21)|(1<<CS21); // clk/1024 (From prescaler)
}

static void stopTimerCustomMacro(void){
    timer2SetPrescaler(TIMERRTC_CLK_STOP); // No clock source (Timer/Counter stopped).  
    // TCCR2 &= ~((1<<CS22)|(1<<CS21)|(1<<CS20)); // No clock source (Timer/Counter stopped).   
}

bool hasCustomMacroAt(uint8_t xMacroIndex){
    uint8_t gKeyidx = 0;

    if(xMacroIndex >= CUSTOM_MACRO_NUM)    // eeprom macro
    {
        gKeyidx = eeprom_read_byte((uint8_t *)(EEPROM_MACRO+(MACRO_SIZE_MAX * (xMacroIndex - CUSTOM_MACRO_NUM))));
    }else{
        gKeyidx = pgm_read_byte((uint8_t*)CUSTOM_MACRO_ADDRESS+(CUSTOM_MACRO_SIZE_MAX * xMacroIndex));
    }
    // 매크로의 첫번째 byte의 값이 있는지 확인;
    if(gKeyidx > 0 && gKeyidx < 255){
        return true;
    }
    return false;
}

// 매크로가 실행되면 timer2를 작동시켜서 정해진 delay마다 매크로 버퍼에 keyindex를 넣어 준다.
// 해당 매크로의 실행이 완료되기 전까지 다른 키입력은 막는다.
void readCustomMacroAt(uint8_t xMacroIndex){
    if(_isActiveCustomMacro) return;

    _isActiveCustomMacro = true;
    _currentMacroIndex = xMacroIndex;
    _customMacroDelay = 0;
    macroCounter = 0;

    pushNextKeyIndex();

    startTimerCustomMacro();
}

void closeCustomMacro(void){
    _isActiveCustomMacro = false;
    stopTimerCustomMacro();
}

static bool isOverSize(void)
{
    if(_currentMacroIndex >= CUSTOM_MACRO_NUM )
    {   // eeprom
        return MACRO_SIZE_MAX <= macroCounter;
    }
    else
    {
        return CUSTOM_MACRO_SIZE_MAX <= macroCounter;
    }
}

static void pushNextKeyIndex(void){
    _countForMs = 0;
    _customMacroDelay = 0;

    uint8_t gKeyindex;
    // isDown/delay
    // 0b10000000 : isDown
    // 0b01111111 : delay * 100 ms
    uint8_t gDownDelay;
//    uint8_t gIsDown;
    uint8_t gDelay;
    while(1){
        if(isOverSize()){
            closeCustomMacro();
            break;
        }

//        DBG1(0x73, (void *)&macroCounter, 1);
        // key index
        if(_currentMacroIndex >= CUSTOM_MACRO_NUM )
        {   // eeprom
            gKeyindex = eeprom_read_byte((uint8_t *)(EEPROM_MACRO + (MACRO_SIZE_MAX * (_currentMacroIndex-CUSTOM_MACRO_NUM)) + macroCounter++));
            gDownDelay = 0;
        }else{
            gKeyindex = pgm_read_byte((uint8_t*)CUSTOM_MACRO_ADDRESS + (CUSTOM_MACRO_SIZE_MAX * _currentMacroIndex) + macroCounter++);
            gDownDelay = pgm_read_byte((uint8_t*)CUSTOM_MACRO_ADDRESS + (CUSTOM_MACRO_SIZE_MAX * _currentMacroIndex) + macroCounter++);
        }
        
//        DBG1(0x74, (void *)&_currentMacroIndex, 1);
//        DBG1(0x75, (void *)&gKeyindex, 1);

        if(gKeyindex > 0 && gKeyindex < 255){

            pushMacroKeyIndex(gKeyindex);
            // 현재 사용되지 않음;
//            gIsDown = (gDownDelay>>7)&0x01;
            // delay : 100ms
            gDelay = gDownDelay & 0x7F; // 10 == 1 secound
            if(gDelay > 0){
                _customMacroDelay = gDelay * COUNT_FOR_100_MS;
                break;
            }
        }else{
            closeCustomMacro();
            stopRepeat();

            break;
        }
    }
    
}

void enterFrameForCustomMacro(void){
    // timer는 최소한의 시간 체크용도로만 사용하고, 나머지 처리는 메인 loop에서 한다.
    if(_isActiveCustomMacro) {
        // DBG1(0x01, (uchar *)&_countForMs, 5);
        if(_countForMs >= _customMacroDelay){
            pushNextKeyIndex();
        }
    }
}

// 매크로 실행 중에는 다른 리포트를 하지 않도록 막는다.
bool isActiveCustomMacro(void){
    return _isActiveCustomMacro;
}

