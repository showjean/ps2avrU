#ifndef KEYINDEX_H
#define KEYINDEX_H

#include <stdio.h>
#include <stdbool.h>


extern uint8_t getKeyIndex(uint8_t xLayer, uint8_t xRow, uint8_t xCol);
extern uint8_t getDefaultKeyindex(uint8_t xLayer, uint8_t xRow, uint8_t xCol);

// 키들을 순서대로 나열한 인덱스를 반환. <키코드가 아님!>
extern uint8_t getCurrentKeyindex(uint8_t xLayer, uint8_t xRow, uint8_t xCol);

#endif