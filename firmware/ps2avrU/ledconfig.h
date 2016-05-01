#ifndef LEDCONFIG_H
#define LEDCONFIG_H

#define PORTLEDS    PORTD  ///< port on which the LEDs are connected
#define PINLEDS     PIND   ///< port on which the LEDs are connected
#define DDRLEDS     DDRD   ///< port on which the LEDs are connected

#define LEDNUM      (1 << 0)	//PIND0  ///< address of the num-lock LED
#define LEDCAPS     (1 << 1)	//PIND1  ///< address of the caps-lock LED
#define LEDSCROLL   (1 << 6)	//PIND6  ///< address of the scroll-lock LED
#define LEDFULLLED  (1 << 4)	//PIND4  ///< address of the full led controll pin

#define turnOnLED(pin)		PORTLEDS |= (pin);
#define turnOffLED(pin)		PORTLEDS &= ~(pin);


#endif
