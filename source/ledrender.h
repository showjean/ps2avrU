#ifndef LEDRENDER_H
#define LEDRENDER_H

// #include "hardwareinfo.h"

#define LED_STATE_NUM     0x01  ///< num LED on a boot-protocol keyboard
#define LED_STATE_CAPS    0x02  ///< caps LED on a boot-protocol keyboard
#define LED_STATE_SCROLL  0x04  ///< scroll LED on a boot-protocol keyboard

extern void setLEDState(uint8_t xState);
extern uint8_t getLEDState(void);

extern void increaseLedBrightness(void);
extern void reduceLedBrightness(void);
extern void changeFullLedState(void);
extern void clearLEDInited(void);

extern void blinkOnce(void);
extern void initFullLEDState(void);
extern void initLED(void);

extern void setLEDIndicate(void);

extern void applyKeyDownForFullLED(void);

extern void sleepLED(void);

extern void wakeUpLED(void);

extern void doSleep(void);
extern void renderLED(uint8_t xIsBeyondFN);

extern void stopTimer(void);
extern void startTimer(void);

#endif