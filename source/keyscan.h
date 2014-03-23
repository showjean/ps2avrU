#ifndef KEYSCAN_H
#define KEYSCAN_H

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    uint8_t (*pushKeyCode)(uint8_t, bool);
    uint8_t (*pushKeyCodeWhenChange)(uint8_t, bool);
} keyscan_driver_t;

void setKeyScanDriver(keyscan_driver_t *driver);
void scanKeyWithDebounce(void);
void scanKeyWithMacro(void);

#endif