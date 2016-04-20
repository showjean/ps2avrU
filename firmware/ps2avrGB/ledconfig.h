#ifndef LEDCONFIG_H
#define LEDCONFIG_H

#include "ledrender.h"

#define PORTLEDS    PORTD  ///< port on which the LEDs are connected
#define PINLEDS     PIND   ///< port on which the LEDs are connected
#define DDRLEDS     DDRD   ///< port on which the LEDs are connected

#define LEDNUM      (1 << 0)	//PIND0  ///< address of the num-lock LED
#define LEDCAPS     (1 << 1)	//PIND1  ///< address of the caps-lock LED
#define LEDSCROLL   (1 << 6)	//PIND6  ///< address of the scroll-lock LED
#define LEDFULLLED  (1 << 4)	//PIND4  ///< address of the full led controll pin

#define turnOnLED(pin)		PORTLEDS |= (pin);
#define turnOffLED(pin)		PORTLEDS &= ~(pin);

#define IS_LIGHT_UP_NL  lockLedStatus.nl == LOCK_LED_ALWAYS_ON \
                        || (lockLedStatus.nl == LOCK_LED_DEFAULT && (LEDstate & LED_STATE_NUM)) \
                        || (lockLedStatus.nl == LOCK_LED_REVERSE && !(LEDstate & LED_STATE_NUM))
#define IS_LIGHT_UP_CL  lockLedStatus.cl == LOCK_LED_ALWAYS_ON \
                        || (lockLedStatus.cl == LOCK_LED_DEFAULT && (LEDstate & LED_STATE_CAPS)) \
                        || (lockLedStatus.cl == LOCK_LED_REVERSE && !(LEDstate & LED_STATE_CAPS))
#define IS_LIGHT_UP_SL  lockLedStatus.sl == LOCK_LED_ALWAYS_ON \
                        || (lockLedStatus.sl == LOCK_LED_DEFAULT && (LEDstate & LED_STATE_SCROLL)) \
                        || (lockLedStatus.sl == LOCK_LED_REVERSE && !(LEDstate & LED_STATE_SCROLL))

#endif
