#ifndef LAZYFN_H
#define LAZYFN_H

#include <stdio.h>
#include <stdbool.h>
#include "keymapper.h"

keymapper_driver_t driverKeymapperLazyFn;

extern void initLazyFn(void);
extern bool isLazyFn(void);
extern void toggleLazyFn(void);

#endif