#ifndef SMARTKEY_H
#define SMARTKEY_H

#include <stdio.h>
#include <stdbool.h>
#include "keymapper.h"

keymapper_driver_t driverKeymapperSmartKey;

void initSmartKey(void);

uint8_t getSmartKeyIndex(uint8_t xKeyidx);

void setCurrentOS(bool xIsOsx);

bool isSmartKeyEnabled(void);
void toggleSmartKeyEnabled(void);


#endif