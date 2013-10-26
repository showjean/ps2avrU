#ifndef HARDWARD_INFO_H
#define HARDWARD_INFO_H

// PB0-PB7 : col1 .. col8
// PA0-PA7 : row1 .. row8
// PC7-PC0 : row9 .. row16

// PD0 : NUM
// PD1 : CAPS
// PD2 : D+ / Clock
// PD3 : D- / Data
// PD4 : FULL LED
// PD5 : 3.6V switch TR
// PD6 : reserved
// PD7 : row17
/* ----------------------- hardware I/O abstraction ------------------------ */
#define PORTCOLUMNS PORTB  ///< port on which we read the state of the columns
#define PINCOLUMNS  PINB   ///< port on which we read the state of the columns
#define DDRCOLUMNS  DDRB   ///< port on which we read the state of the columns
#define PORTROWS1   PORTA  ///< first port connected to the matrix rows
#define PINROWS1    PINA   ///< first port connected to the matrix rows
#define DDRROWS1    DDRA   ///< first port connected to the matrix rows
#define PORTROWS2   PORTC  ///< second port connected to the matrix rows
#define PINROWS2    PINC   ///< second port connected to the matrix rows
#define DDRROWS2    DDRC   ///< second port connected to the matrix rows

#define PORTLEDS    PORTD  ///< port on which the LEDs are connected
#define PINLEDS     PIND   ///< port on which the LEDs are connected
#define DDRLEDS     DDRD   ///< port on which the LEDs are connected

#define DIODE_DDR	DDRD
#define DIODE_PORT	PORTD

#define LEDNUM      (1 << 0)	//PIND0  ///< address of the num-lock LED
#define LEDCAPS     (1 << 1)	//PIND1  ///< address of the caps-lock LED
#define LEDSCROLL   (1 << 6)	//PIND6  ///< address of the scroll-lock LED  // if define, Scroll lock LED is toggle by PD6
#define LEDFULLLED  (1 << 4)	//PIND4  ///< address of the full led controll pin
#define DIODE_PIN	5	 		// 제너 다이오드를 컨트롤할 D 핀 중에 하나;
//DATA+(clk) 	2
//DATA-(dat) 	3

#define INTERFACE_PS2		0
#define INTERFACE_PS2_USER	3
#define INTERFACE_USB		1
#define INTERFACE_USB_USER	2
#define INTERFACE_CLEAR		4

#define EEPROM_LED_BRIGHTNESS 	9	// 1byte
#define EEPROM_LED_MODE			11	// 1byte
#define EEPROM_INTERFACE		12	// 1byte
#define EEPROM_MAPPING			13	// ~420
#define EEPROM_MACRO			421	// ~804

extern int interfaceCount;
extern uint8_t interfaceReady;

extern uint8_t INTERFACE;		// ps/2 : 0, usb : 1, user usb : 2, user ps/2 : 3, clear user interface : 4 

#endif