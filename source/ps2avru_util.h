#ifndef PS2AVRU_UTIL_H
#define PS2AVRU_UTIL_H

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

void insert(uint8_t ary[], int idx, char ch);
void delete(uint8_t ary[], int idx);
void append(uint8_t ary[], char ch);
 
int findIndex(uint8_t xArr[], uint8_t xValue);

void __delay_ms(int n);

void setToggleOption(int xAddress, uint8_t xBit, bool xBool);

bool getToggleOption(int xAddress, uint8_t xBit);

#endif