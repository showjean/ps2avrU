#ifndef DUALACTION_H
#define DUALACTION_H

#include <stdbool.h>
#include <avr/pgmspace.h>

extern void setDualAction(uint8_t keyidx, bool isDown);
// fncontrol에 사용되는 기본 키를 반환한다.
extern uint8_t getDualActionDefaultKey(uint8_t xActionIndex);
extern uint8_t getDualActionKeyWhenCompound(uint8_t keyidx);

// extern void enterFrameForDualAction(void);

#endif
