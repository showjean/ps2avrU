#ifndef KEYMATRIX_H
#define KEYMATRIX_H

/* ------------------------------------------------------------------------- */
/* -----------------------------    variable  global ----------------------------- */
/* ------------------------------------------------------------------------- */

extern uint8_t prevMatrix[ROWS];

/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  global ----------------------------- */
/* ------------------------------------------------------------------------- */
extern void initMatrix(void);

extern void clearMatrix(void);

extern uint8_t isAllKeyRelease(void);

// function that determine keymap
// 0 = normal, 1 = fn, 2 = beyond_fn
extern uint8_t getLayer(void);

extern uint8_t getLiveMatrix(void);
//curmatrix
extern uint8_t setCurrentMatrix(void);
extern uint8_t *getCurrentMatrix(void);


#endif
