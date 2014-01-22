#ifndef KEYMATRIX_H
#define KEYMATRIX_H

#include <stdbool.h>

/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  global ----------------------------- */
/* ------------------------------------------------------------------------- */
extern void initMatrix(void);
extern void initMatrixDevice(void);

extern void clearMatrix(void);
extern void clearPrevMatrix(void);

extern bool isReleaseAll(void);
extern bool isReleaseAllPrev(void);

// function that determine keymap
// 0 = normal, 1 = fn, 2 = beyond_fn
extern uint8_t getLayer(void);

extern uint8_t getLiveMatrix(void);
//curmatrix
extern uint8_t setCurrentMatrix(void);
extern uint8_t *getCurrentMatrix(void);
extern void setPrevMatrix(void);
extern uint8_t *getPrevMatrix(void);

extern void setCellStatus(uint8_t xCol);
extern uint8_t getCellStatus(uint8_t xRow);


#endif
