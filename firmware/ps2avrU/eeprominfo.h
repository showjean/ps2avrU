#ifndef EEPROMINFO_H
#define EEPROMINFO_H

// EEPROM_ENABLED_OPTION
#define TOGGLE_LAZY_FN		0
#define TOGGLE_SMART_KEY	1

// eeprom address
#define EEPROM_ENABLED_OPTION 	7	// 1byte
#define EEPROM_QUICK_SWAP	 	8	// 1byte
#define EEPROM_LED_BRIGHTNESS 	9	// 1byte
#define EEPROM_LED_MODE			11	// 1byte
#define EEPROM_INTERFACE		12	// 1byte
#define EEPROM_MAPPING			13	// ~420
#define EEPROM_MACRO			421	// ~996

#endif