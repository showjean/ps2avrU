#ifndef EEPROMINFO_GB_H
#define EEPROMINFO_GB_H

// OPTION FLAG
#define OPTION_ON			0
#define OPTION_OFF			1

// EEPROM_ENABLED_OPTION bit
#define TOGGLE_LAZY_FN		    0
#define TOGGLE_SMART_KEY        1
#define TOGGLE_BEYOND_FN_LED_NL 2
#define TOGGLE_ESC_TO_TILDE	    3
#define TOGGLE_BEYOND_FN_LED_SL 4

// eeprom address
#define EEPROM_BOOTLOADER_START	1	// 1byte
#define EEPROM_LED2_BRIGHTNESS	2	// 1byte
#define EEPROM_LED2_KEY_EVENT	3	// 1byte	0xF0 : type, 0x0F : mode
#define EEPROM_LED2_NUM			4	// 1byte
#define EEPROM_LED2_FADE_TYPE	5	// 1byte
// 6 : 1bytes
#define EEPROM_ENABLED_OPTION 	7	// 1byte
#define EEPROM_QUICK_SWAP	 	8	// 1byte
#define EEPROM_LED_BRIGHTNESS 	9	// 1byte
#define EEPROM_LED_MODE			11	// 1byte	0xF0 : led2, 0x0F : full led
#define EEPROM_INTERFACE		12	// 1byte
#define EEPROM_MAPPING			13	// 360byte ~372
#define EEPROM_LED2_COLOR_RAINBOW 373	// 21byte ~393
#define EEPROM_LED2_COLOR_1		394	// 3byte
#define EEPROM_LED2_COLOR_2		397	// 3byte
#define EEPROM_LED2_COLOR_3		400	// 3byte ~402
// 403~420 : 18bytes
#define EEPROM_MACRO			421	// ~996
// 1000~1024 : 25bytes
#define EEPROM_LED2_COLOR_KEY1	997	// 3byte ~999

#endif
