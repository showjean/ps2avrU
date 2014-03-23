#ifndef KEYMATRIX_H
#define KEYMATRIX_H

#include <stdbool.h>

#define LAYER_NORMAL		0
#define LAYER_FN			1
#define LAYER_FN2			2
#define LAYER_FN3			3

/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  global ----------------------------- */
/* ------------------------------------------------------------------------- */
void initMatrix(void);
extern void delegateInitMatrixDevice(void);

void clearMatrix(void);

bool isReleaseAll(void);
bool isReleaseAllPrev(void);
bool isFnPressed(void);

// function that determine keymap
// 0 = normal, 1 = fn, 2 = beyond_fn
uint8_t getLayer(void);

uint8_t getLiveMatrix(void);
//curmatrix
void setCurrentMatrixAfter(void);
uint8_t setCurrentMatrix(void);
uint8_t *getCurrentMatrix(void);
void setPrevMatrix(void);
uint8_t *getPrevMatrix(void);

extern void delegateSetCellStatus(uint8_t xCol);
extern uint8_t delegateGetCellStatus(uint8_t xRow);


#endif
