#ifndef KEYSCAN_H
#define KEYSCAN_H

#include <stdio.h>
#include <stdbool.h>

// #define RETURN_NONE		0
// #define RETURN_CONTINUE	1
// #define RETURN_RETURN	2

typedef struct {
    uint8_t (*pushKeyCode)(uint8_t, bool);
    uint8_t (*pushKeyCodeWhenDown)(uint8_t, bool);
    uint8_t (*pushKeyCodeWhenChange)(uint8_t, bool);
} keyscan_driver_t;

extern void setKeyScanDriver(keyscan_driver_t *driver);
// extern uint8_t putChangedKey(uint8_t keyidx, bool isDown, uint8_t col, uint8_t row);
extern uint8_t processKeyIndex(uint8_t xKeyidx, bool xPrev, bool xCur, uint8_t xCol, uint8_t xRow);
extern uint8_t scanKey(uint8_t xLayer);

#endif