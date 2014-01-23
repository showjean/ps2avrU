#ifndef PS2AVRU_UTIL_H
#define PS2AVRU_UTIL_H

#include <string.h>
#include <stdio.h>

extern void insert(uint8_t ary[], int idx, char ch);
extern void delete(uint8_t ary[], int idx);
extern void append(uint8_t ary[], char ch);
 
extern int findIndex(uint8_t xArr[], uint8_t xlen, uint8_t xValue);

// extern int equalAll(uint8_t xArr[], uint8_t xlen);

#endif