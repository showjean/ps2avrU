#ifndef ESC_TILDE_H
#define ESC_TILDE_H

#include <stdio.h>
#include <stdbool.h>
#include "keymapper.h"

//keymapper_driver_t driverKeymapperEscTilde;
void initEscTilde(void);
bool isEscTilde(void);
void toggleEscTilde(void);
uint8_t getEscToTilde(uint8_t xKeyidx, bool xIsDown);

#endif
