#ifndef KEYINDEX_H
#define KEYINDEX_H

#include <stdio.h>
#include <stdbool.h>


uint8_t getKeyIndex(uint8_t xLayer, uint8_t xRow, uint8_t xCol);
uint8_t getDefaultKeyindex(uint8_t xLayer, uint8_t xRow, uint8_t xCol);

// 키들을 순서대로 나열한 인덱스를 반환. <키코드가 아님!>
uint8_t getCurrentKeyindex(uint8_t xLayer, uint8_t xRow, uint8_t xCol);

// 각종 옵션에 의해 변환되는 키 인덱스를 반환;
uint8_t getExchangedKeyindex(uint8_t xKeyindex);

#endif