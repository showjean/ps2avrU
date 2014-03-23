#ifndef DUALACTION_H
#define DUALACTION_H

#include <stdbool.h>
#include <avr/pgmspace.h>

void setDualAction(uint8_t keyidx, bool isDown);
// fncontrol에 사용되는 기본 키를 반환한다.
uint8_t getDualActionDefaultKey(uint8_t xActionIndex);
uint8_t getDualActionKeyWhenCompound(uint8_t keyidx);

// void enterFrameForDualAction(void);

#endif
