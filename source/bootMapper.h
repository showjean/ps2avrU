#ifndef BOOTMAPPER_H
#define BOOTMAPPER_H

#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <stdbool.h>

#ifdef ENABLE_BOOTMAPPER
// bootmapper
void trace(uint8_t xRow, uint8_t xCol);
void setToBootMapper(bool xBool);
bool isBootMapper(void);
#endif

#endif
