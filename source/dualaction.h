#ifndef DUALACTION_H
#define DUALACTION_H

#include <stdbool.h>
#include <avr/pgmspace.h>

void applyDualActionDownWhenIsCompounded(bool isDown);
void setDualAction(uint8_t keyidx, bool isDown);
void clearDualAction(void);

// fncontrol에 사용되는 기본 키를 반환한다.
uint8_t getDualActionDefaultKeyForFncontrol(uint8_t xActionIndex);
uint8_t getDualActionDownKeyIndexWhenIsCompounded(uint8_t xActionIndex, bool xForceCompounded);

 void enterFrameForDualAction(void);

#endif
