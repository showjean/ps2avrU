#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdio.h>
#include <stdbool.h>
#include "version.h"

#define OPTION_INDEX_COLOR_RAINBOW_INIT	0x1F
#define OPTION_INDEX_COLOR_RAINBOW	0x10
#define OPTION_INDEX_COLOR_SET1		0x11
#define OPTION_INDEX_COLOR_SET2		0x12
#define OPTION_INDEX_COLOR_SET3		0x13
#define OPTION_INDEX_MODE				0x20
#define OPTION_INDEX_MODE_KEY			0x21
#define OPTION_INDEX_MODE_KEY_TYPE	0x22
#define OPTION_INDEX_MODE_FADE_TYPE	0x23
#define OPTION_INDEX_LED_NUM			0x30
#define OPTION_INDEX_LED_BRIGHTNESS	0x40
#define OPTION_INDEX_COLOR_KEY_SET	0xA0
#define OPTION_INDEX_COLOR_KEY_SET1	0xA1
#define OPTION_INDEX_LED_ENABLED		0xE0
#define OPTION_INDEX_LED_SAVE         0xF0
#define OPTION_INDEX_FULL_LED_MODE    0x80
#define OPTION_INDEX_FULL_LED_BRIGHTNESS 0x81
#define OPTION_INDEX_ESC_TO_TILDE     0x82
#define OPTION_INDEX_FN_LED           0x83
#define OPTION_INDEX_TRANSITION_DELAY 0x84
#define OPTION_INDEX_LOCK_LED_NL    0x85
#define OPTION_INDEX_LOCK_LED_CL    0x86
#define OPTION_INDEX_LOCK_LED_SL    0x87
#define OPTION_INDEX_NUMLOCK_LAYER  0x88

#define OPTION_GET_REPORT_LENGTH_RAINBOW			133	//0x85
#define OPTION_GET_REPORT_LENGTH_INFO				100
#define OPTION_GET_REPORT_LENGTH_TOGGLE_BOOTMAPPER  101
#if HAS_OPTIONS
    #define OPTION_GET_REPORT_LENGTH_KEYMAP			120
    #define OPTION_GET_REPORT_LENGTH_KEYMAP_LAYER1	121
    #define OPTION_GET_REPORT_LENGTH_KEYMAP_LAYER2	122
    #define OPTION_GET_REPORT_LENGTH_KEYMAP_LAYER3	123
    #define OPTION_GET_REPORT_LENGTH_KEYMAP_LAYER4	124
#else
    #define OPTION_GET_REPORT_LENGTH_KEYMAP         136
    #define OPTION_GET_REPORT_LENGTH_KEYMAP_LAYER1  137
    #define OPTION_GET_REPORT_LENGTH_KEYMAP_LAYER2  138
    #define OPTION_GET_REPORT_LENGTH_KEYMAP_LAYER3  139
    #define OPTION_GET_REPORT_LENGTH_KEYMAP_LAYER4  140
#endif


#define OPTION_GET_OPTION_INDEX_DUALACTION  150

#define OPTION_GET_REPORT_LENGTH_MACRO1  168
enum{
	OPTION_GET_REPORT_LENGTH_MACRO2 = 169,
	OPTION_GET_REPORT_LENGTH_MACRO3,
	OPTION_GET_REPORT_LENGTH_MACRO4,
	OPTION_GET_REPORT_LENGTH_MACRO5,
	OPTION_GET_REPORT_LENGTH_MACRO6,
	OPTION_GET_REPORT_LENGTH_MACRO7,
	OPTION_GET_REPORT_LENGTH_MACRO8,
	OPTION_GET_REPORT_LENGTH_MACRO9,
	OPTION_GET_REPORT_LENGTH_MACRO10,
	OPTION_GET_REPORT_LENGTH_MACRO11	//178
};
#define OPTION_GET_REPORT_LENGTH_MACRO12  179

#define OPTION_GET_REPORT_LENGTH_QUICK_MACRO  180
#define OPTION_GET_REPORT_LENGTH_QUICK_MACRO1  180
enum{
    OPTION_GET_REPORT_LENGTH_QUICK_MACRO2 = 181,
    OPTION_GET_REPORT_LENGTH_QUICK_MACRO3,
    OPTION_GET_REPORT_LENGTH_QUICK_MACRO4,
    OPTION_GET_REPORT_LENGTH_QUICK_MACRO5,
    OPTION_GET_REPORT_LENGTH_QUICK_MACRO6,
    OPTION_GET_REPORT_LENGTH_QUICK_MACRO7,
    OPTION_GET_REPORT_LENGTH_QUICK_MACRO8,
    OPTION_GET_REPORT_LENGTH_QUICK_MACRO9,
    OPTION_GET_REPORT_LENGTH_QUICK_MACRO10,
    OPTION_GET_REPORT_LENGTH_QUICK_MACRO11,     // 190
};
#define OPTION_GET_REPORT_LENGTH_QUICK_MACRO12  191

#define OPTION_INDEX_BOOTLOADER		0x00
#define OPTION_INDEX_BOOTMAPPER		0x01
#define OPTION_INDEX_READY			0x90
#define OPTION_INDEX_ACTION			0x91

#define OPTION_VALUE_BOOTMAPPER_START		0x01
#define OPTION_VALUE_BOOTMAPPER_STOP		0x00

/*
 *  Structure of the LED array
 */
//struct cRGB { uint8_t g; uint8_t r; uint8_t b; };
typedef struct {
	uint8_t g;
	uint8_t r;
	uint8_t b;
} cRGB_t;

/**
 *  0. indicator for lock status(default)
    1. always on
    2. always off
 */
#define LOCK_LED_DEFAULT    0
#define LOCK_LED_ALWAYS_ON  1
#define LOCK_LED_ALWAYS_OFF 2
#define LOCK_LED_REVERSE    3
#define LOCK_LED_FN2_TOGGLE  4
#define LOCK_LED_FN3_TOGGLE  5
#define LOCK_LED_FN23_TOGGLE  6

typedef struct {
    uint8_t nl;
    uint8_t cl;
    uint8_t sl;
} lock_led_t;

typedef struct {
	// num 1byte, mode 1byte, brightness 1byte, color1 3byte, color2 3byte, color3 3byte, rainbow colors 21byte, keymode 1byte, key color1 3byte
    // fade type 1byte,
    // = 38 bytes

    // Ver1.1
    //skipFrame 1byte, full led mode 1byte, full led brightness 1byte, esc to tilde 1byte, fn led 1byte
    // = 5 bytes

    // Ver 1.2
    // data size 1byte
    //ver 3byte, firm 1byte, lockled 3byte
    // = 7 bytes
    // ver 1.2.4
    // numlocklayer 1byte
    // = 1 byte

    uint8_t size;
    uint8_t num;
    uint8_t mode;
    uint8_t brightness;
    cRGB_t color1;
    cRGB_t color2;
    cRGB_t color3;
    cRGB_t rainbow[7];
    uint8_t keymode;
    cRGB_t colorkey1;
    uint8_t fadetype;

    //Ver1.1
    uint8_t transitiondelay;

    uint8_t fullledmode;
    uint8_t fullledbrightness;

    uint8_t esctotilde;
    uint8_t fnled;

    //Ver 1.2
    uint8_t version[3];    // r/o, 1.2.0
    uint8_t firmware;      // r/o, 0= ps2avrGB, 1=ps2varGB4U ...
    lock_led_t lockled;
    uint8_t numlocklayer;      //


} option_info_t;

void setOptions(uint8_t *data);
void getOptions(option_info_t *buffer);

void stopPwmForUsbReport(bool xIsStop);

#endif
