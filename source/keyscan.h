#ifndef KEYSCAN_H
#define KEYSCAN_H

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    uint8_t (*pushKeyCode)(uint8_t, bool);
    uint8_t (*pushKeyCodeWhenDown)(uint8_t, bool);
    uint8_t (*pushKeyCodeWhenChange)(uint8_t, bool);
} keyscan_driver_t;

extern void setKeyScanDriver(keyscan_driver_t *driver);
extern uint8_t putChangedKey(uint8_t keyidx, bool isDown, uint8_t col, uint8_t row);
extern uint8_t scanKey(uint8_t xLayer);

#endif