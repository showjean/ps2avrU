#ifndef KEYMATRIX_H
#define KEYMATRIX_H

#define EXTRA_FN				KEY_ESC		// led 모드변경과 beyond_fn 취소를 위한 조합키
#define LED_KEY					KEY_CAPS	// led 모드 변경 키 (EXTRA_FN 와 조합)
#define BEYOND_FN_CANCEL_KEY	KEY_BKSP	// 비욘드FN 취소키 (EXTRA_FN 와 조합)

/* ------------------------------------------------------------------------- */
/* -----------------------------    variable  global ----------------------------- */
/* ------------------------------------------------------------------------- */

// for KEY_BEYOND_FN;
extern uint8_t isBeyondFN;//KEY_BEYOND_FN state
extern const unsigned short int modmask[16];
extern uint8_t prevMatrix[ROWS];

/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  global ----------------------------- */
/* ------------------------------------------------------------------------- */
extern void initMatrix(void);

extern void clearMatrix(void);

extern uint8_t isAllKeyRelease(void);

// 키를 누르거나 땔때 FN 및 LED등 을 컨트롤한다.
extern uint8_t applyFN(uint8_t keyidx, uint8_t isDown);

// function that determine keymap
// 0 = normal, 1 = fn, 2 = beyond_fn
extern uint8_t getLayer(void);

extern uint8_t getLiveMatrix(void);
//curmatrix
extern uint8_t setCurrentMatrix(void);
extern uint8_t *getCurrentMatrix(void);


#endif
