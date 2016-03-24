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
// 0 = normal, 1 = fn, 2 = beyond_fn
uint8_t getLayer(void);
bool isFnPosition(uint8_t xCol, uint8_t xRow);
uint8_t getFnScanLayer(void);

uint8_t getLiveMatrix(void);
//curmatrix
void setCurrentMatrixAfter(void);
uint8_t setCurrentMatrix(void);
uint8_t *getCurrentMatrix(void);
//void setPrevMatrix(void);
uint8_t *getPrevMatrix(void);

extern void delegateGetLiveMatrix(uint8_t *xCurrentMatrix, uint8_t *isModified);
//extern void delegateSetCellStatus(uint8_t xCol);
//extern uint8_t delegateGetCellStatus(uint8_t xRow);


#endif
