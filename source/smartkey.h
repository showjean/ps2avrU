#ifndef SMARTKEY_H
#define SMARTKEY_H

#include <stdio.h>
#include <stdbool.h>

extern void initSmartKey(void);

extern uint8_t getSmartKeyIndex(uint8_t xKeyidx);

extern void setCurrentOS(bool xIsOsx);

extern bool isSmartKeyEnabled(void);
extern void toggleSmartKeyEnabled(void);


#endif