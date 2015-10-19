#ifndef ESC_TILDE_H
#define ESC_TILDE_H

#include <stdio.h>
#include <stdbool.h>
#include "keymapper.h"

#ifndef DISABLE_HARDWARE_MENU
void toggleEscTilde(void);
#endif

void initEscTilde(void);
bool isEscTilde(void);
void setEscTilde(bool xEnabled);
uint8_t getEscToTilde(uint8_t xKeyidx, bool xIsDown);

#endif
