#ifndef KEYMATRIX_H
#define KEYMATRIX_H

#include <stdbool.h>

#define LAYER_NORMAL		0
#define LAYER_FN			1
#define LAYER_FN2			2
#define LAYER_FN3			3
#define LAYER_NOR           10
#define LAYER_NOTHING       99

/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  global ----------------------------- */
/* ------------------------------------------------------------------------- */
void initMatrix(void);
extern void delegateInitMatrixDevice(void);

void clearMatrix(void);

bool isReleaseAll(void);

// function that determine keymap
// 0 = normal, 1 = fn, 2 = fn2, 3 = fn3
uint8_t getLayer(void);
bool isFnPosition(uint8_t xCol, uint8_t xRow);
void clearFnPosition(void);
void setFnPressed(uint8_t xFnIndex);

uint8_t getLiveMatrix(void);
void setCurrentMatrixAfter(void);
uint8_t setCurrentMatrix(void);
uint8_t *getCurrentMatrix(void);
uint8_t *getPrevMatrix(void);

extern void delegateGetLiveMatrix(uint8_t *xCurrentMatrix, uint8_t *isModified);


#endif
