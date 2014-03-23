#ifndef SMARTKEY_H
#define SMARTKEY_H

#include <stdio.h>
#include <stdbool.h>
#include "keymapper.h"

keymapper_driver_t driverKeymapperSmartKey;

extern void initSmartKey(void);

extern uint8_t getSmartKeyIndex(uint8_t xKeyidx);

extern void setCurrentOS(bool xIsOsx);

extern bool isSmartKeyEnabled(void);
extern void toggleSmartKeyEnabled(void);


#endif