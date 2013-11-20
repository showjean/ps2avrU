#ifndef BOOTMAPPER_H
#define BOOTMAPPER_H

#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#ifdef ENABLE_BOOTMAPPER
// bootmapper
extern void trace(uint8_t xRow, uint8_t xCol);
extern void setToBootMapper(void);
extern uint8_t isBootMapper(void);
#endif

#endif