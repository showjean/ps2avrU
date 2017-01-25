#ifndef LEDRENDER_H
#define LEDRENDER_H

#include <stdbool.h>
#include <stdio.h>
#include "hardwareinfo.h"

#define LED_STATE_NUM     0x01  ///< num LED on a boot-protocol keyboard
#define LED_STATE_CAPS    0x02  ///< caps LED on a boot-protocol keyboard
#define LED_STATE_SCROLL  0x04  ///< scroll LED on a boot-protocol keyboard

#define FULL_LED_MODE1	1
#define FULL_LED_MODE2	2

#define PWM_MAX             0xFF

void initLED(void);
void clearLEDInited(void);
void setLEDState(uint8_t xState);
uint8_t getLEDState(void);
void setLEDIndicate(void);
void setLed(uint8_t xLed, bool xBool);
void blinkOnce(const int xStayMs);

void increaseLedBrightness(uint8_t xFullLedMode);
void reduceLedBrightness(uint8_t xFullLedMode);
void changeFullLedState(uint8_t xFullLedMode);

void initFullLEDState(void);
void initFullLEDStateAfter(void);

void applyKeyDownForFullLED(uint8_t keyidx, uint8_t col, uint8_t row, uint8_t isDown);

void sleepLED(void);
void wakeUpLED(void);
void renderLED(void);
void setPWM(int xValue);

void turnOnLedAll(void);
void turnOffLedAll(void);

#endif
