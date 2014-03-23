#ifndef PS2AVRU_UTIL_C
#define PS2AVRU_UTIL_C

#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "ps2avru_util.h"
#include "eeprominfo.h"

void insert(uint8_t ary[], int idx, char ch)
{
     memmove(ary + idx + 1, ary + idx, strlen((char *)ary) - idx + 1);
     ary[idx] = ch;
}
 
void delete(uint8_t ary[], int idx)
{
     memmove(ary + idx, ary + idx + 1, strlen((char *)ary) - idx);
}
 
void append(uint8_t ary[], char ch)
{
     insert(ary, strlen((char *)ary), ch);
}
 
int findIndex(uint8_t xArr[], uint8_t xValue)
{
	char * pch;
    pch = (char*) memchr((char *)xArr, xValue, strlen((char *)xArr));

    if (pch!=NULL)
        return (pch-(char *)xArr);
    else
        return -1;
}
/*
    _delay_ms()에 xStayMs를 인자로 넣으면 hex 파일의 용량이 0x1000가량 증가한다. 
    매뉴얼 펑션으로 _delay_ms(1)을 ms 만큼 루프시키도록 만들어서 사용;
*/
void __delay_ms(int n) { 
  while (--n) { 
   _delay_ms(1); 
 } 
}

void setToggleOption(int xAddress, uint8_t xBit, bool xBool){
    uint8_t gOption = eeprom_read_byte((uint8_t *)xAddress);
    if(xBool == false){
        gOption = (gOption & ~(1<<xBit)) | (OPTION_OFF<<xBit);
        // gOption &= ~(1<<xBit);
    }else{
        gOption = (gOption & ~(1<<xBit)) | (OPTION_ON<<xBit);
        // gOption |= (1<<xBit);
    }
    eeprom_write_byte((uint8_t *)EEPROM_ENABLED_OPTION, gOption);
}

bool getToggleOption(int xAddress, uint8_t xBit){

    uint8_t gOption = eeprom_read_byte((uint8_t *)xAddress);
    if(((gOption >> xBit) & 0x01) == OPTION_OFF){
        return false;
    }else{
        return true;
    }
}

#endif