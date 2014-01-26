#ifndef LEDRENDER_H
#define LEDRENDER_H

#include "hardwareinfo.h"

#define LED_STATE_NUM     0x01  ///< num LED on a boot-protocol keyboard
#define LED_STATE_CAPS    0x02  ///< caps LED on a boot-protocol keyboard
#define LED_STATE_SCROLL  0x04  ///< scroll LED on a boot-protocol keyboard

#define turnOnLED(pin)		PORTLEDS |= (pin);
#define turnOffLED(pin)		PORTLEDS &= ~(pin);

extern void setLEDState(uint8_t xState);
extern uint8_t getLEDState(void);

extern void increaseLedBrightness(void);
extern void reduceLedBrightness(void);
extern void changeFullLedState(void);
extern void clearLEDInited(void);

extern void blinkOnce(int xStayMs);
extern void initFullLEDState(void);
extern void initFullLEDStateAfter(void);
extern void initLED(void);

extern void setLEDIndicate(void);

extern void applyKeyDownForFullLED(uint8_t keyidx, uint8_t col, uint8_t row, uint8_t isDown);

extern void sleepLED(void);

extern void wakeUpLED(void);

extern void renderLED(void);

extern void stopTimer(void);
extern void startTimer(void);
extern void setPWM(int xValue);

#endif