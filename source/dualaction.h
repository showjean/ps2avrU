#ifndef DUALACTION_H
#define DUALACTION_H

#include <stdbool.h>
#include <avr/pgmspace.h>

extern void applyDualActionDownWhenIsCancel(uint8_t (*func)(uint8_t, bool), bool isDown);
extern void setDualAction(uint8_t keyidx, bool isDown);
// extern bool isCanceledDualAction(void);
extern uint8_t getDualActionDefaultKeyIndexWhenIsCancel(uint8_t xActionIndex);
extern uint8_t getDualActionDownKeyIndexWhenIsCancel(uint8_t xActionIndex);
extern uint8_t getDualActionMaskDown(uint8_t keyidx);

#endif
