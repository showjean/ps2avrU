#ifndef NUMBERLAYER_H
#define NUMBERLAYER_H

#include <stdio.h>
#include <stdbool.h>
#include "eeprominfo.h"

void initNumlockLayer(void);
uint8_t getNumlockLayer(void);
void setNumlockLayer(uint8_t xNumlockLayer);

#endif
