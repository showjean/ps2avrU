#ifndef EEPROMINFO_H
#define EEPROMINFO_H

// OPTION FLAG
#define OPTION_ON			0
#define OPTION_OFF			1

// EEPROM_ENABLED_OPTION bit
#define TOGGLE_LAZY_FN		    0
#define TOGGLE_SMART_KEY        1
#define TOGGLE_BEYOND_FN_LED_NL 2
#define TOGGLE_ESC_TO_TILDE	    3
#define TOGGLE_BEYOND_FN_LED_SL 4

#define EEPROM_PS2_REPEAT_SPEED	6	// 1byte
// eeprom address
#define EEPROM_ENABLED_OPTION 	7	// 1byte
#define EEPROM_QUICK_SWAP	 	8	// 1byte
#define EEPROM_LED_BRIGHTNESS 	9	// 1byte
#define EEPROM_LED_MODE			11	// 1byte
#define EEPROM_INTERFACE		12	// 1byte
#define EEPROM_MAPPING			13	// ~420
#define EEPROM_MACRO			421	// ~996

#endif
