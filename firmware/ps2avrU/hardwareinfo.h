#ifndef HARDWARE_INFO_H
#define HARDWARE_INFO_H

#include "eeprominfo.h"

// PB0-PB7 : col1 .. col8
// PA0-PA7 : row1 .. row8
// PC7-PC0 : row9 .. row16

// PD0 : LED NUM
// PD1 : LED CAPS
// PD2 : D+ / Clock
// PD3 : D- / Data
// PD4 : FULL LED
// PD5 : 3.6V switch TR
// PD6 : LED SCROLL
// PD7 : row17

// ----------------------------------------------------------------------------------
// #define DISABLED_TR_SWITCH
#define DIODE_DDR	DDRD
#define DIODE_PORT	PORTD
#define DIODE_PIN	5	 		// 제너 다이오드를 컨트롤할 D 핀 중에 하나;

#define LAYERS		3
#define COLUMNS 	8
#define ROWS 		17

#define KEYMAP_ADDRESS			0x6500
#define CUSTOM_MACRO_ADDRESS	0x6800

#endif