#ifndef FNCONTROL_H
#define FNCONTROL_H

#include <stdio.h>
#include <stdbool.h>

#define EXTRA_FN				KEY_ESC		// led 모드변경과 beyond_fn 취소를 위한 조합키
#define LED_KEY					KEY_CAPS	// led 모드 변경 키 (EXTRA_FN 와 조합)
#define BEYOND_FN_CANCEL_KEY	KEY_BKSP	// 비욘드FN 취소키 (EXTRA_FN 와 조합)

/* ------------------------------------------------------------------------- */
/* -----------------------------    variable  global ----------------------------- */
/* ------------------------------------------------------------------------- */

// for KEY_BEYOND_FN;
extern bool isBeyondFN(void);//KEY_BEYOND_FN state

// 키를 누르거나 땔때 FN 및 LED등 을 컨트롤한다.
extern bool applyFN(uint8_t keyidx, uint8_t col, uint8_t row, bool isDown);

extern bool isKeyEnabled(uint8_t xKeyidx, bool xIsDown);

#endif
