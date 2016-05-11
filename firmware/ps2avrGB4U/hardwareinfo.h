#ifndef HARDWARE_INFO_H_GB
#define HARDWARE_INFO_H_GB

#include "eeprominfo.h"
#include "version.h"


// for test
//#define USING_SIMPLE_MODE

#define DISABLE_HARDWARE_MENU

#if defined(USING_SIMPLE_MODE)
#warning "=============================================================================================================="
#warning "Never compile production devices with USING_SIMPLE_MODE enabled"
#warning "=============================================================================================================="
#endif


#define FIRMWARE        1   // 0=ps2avrGB, 1=ps2avrGB4U


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

#define HARDWARE_KEYMAPPING_LAYER_MAX	3

#define LAYERS		4
#define COLUMNS 	8
#define ROWS 		17

// dualaction
#define DUALACTION_BYTES     0x80    // 128

// flash address
#define DUALACTION_ADDRESS      0x6480
#define KEYMAP_ADDRESS          0x6500
#define CUSTOM_MACRO_ADDRESS    0x6800

// eeprom macro
#define MACRO_SIZE_MAX 48
#define MACRO_SIZE_MAX_HALF 24
#define MACRO_NUM 12

// flash macro
/*
	2048 flash space
	12 macros
	42 keycodes per macro
*/
#define CUSTOM_MACRO_NUM 12
#define CUSTOM_MACRO_SIZE_MAX 			168 // = 42 * 4

#endif
