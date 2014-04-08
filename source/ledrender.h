#ifndef LEDRENDER_H
#define LEDRENDER_H

#include <stdbool.h>
#include <stdio.h>
#include "hardwareinfo.h"

#define LED_STATE_NUM     0x01  ///< num LED on a boot-protocol keyboard
#define LED_STATE_CAPS    0x02  ///< caps LED on a boot-protocol keyboard
#define LED_STATE_SCROLL  0x04  ///< scroll LED on a boot-protocol keyboard

void setLEDState(uint8_t xState);
uint8_t getLEDState(void);

void increaseLedBrightness(void);
void reduceLedBrightness(void);
void changeFullLedState(void);
void clearLEDInited(void);

void blinkOnce(const int xStayMs);
void initFullLEDState(void);
void initFullLEDStateAfter(void);
void initLED(void);

void setLEDIndicate(void);
void setLed(uint8_t xLed, bool xBool);

void applyKeyDownForFullLED(uint8_t keyidx, uint8_t col, uint8_t row, uint8_t isDown);

void sleepLED(void);

void wakeUpLED(void);

void renderLED(void);

void stopTimer(void);
void startTimer(void);
void setPWM(int xValue);

#endif