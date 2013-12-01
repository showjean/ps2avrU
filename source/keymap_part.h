#ifndef KEYMAP_KEYMAP_PART_H
#define KEYMAP_KEYMAP_PART_H

#include <avr/pgmspace.h>
#include "hardwareinfo.h"
#include "keymap.h"

#define KEY_MATRIX_BASIC __attribute__ ((section (".key_matrix_basic")))
//
extern const uint8_t KEY_MATRIX_BASIC keymap_code[LAYERS][ROWS][COLUMNS];


#endif
