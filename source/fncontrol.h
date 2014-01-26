#ifndef FNCONTROL_H
#define FNCONTROL_H

#include <stdio.h>
#include <stdbool.h>

#define EXTRA_FN				KEY_ESC		// led 모드변경과 beyond_fn 취소를 위한 조합키
#define LED_KEY					KEY_CAPS	// led 모드 변경 키 (EXTRA_FN 와 조합)
#define BEYOND_FN_CANCEL_KEY	KEY_BKSP	// 비욘드FN 취소키 (EXTRA_FN 와 조합)

#define LOCK_NOT_SET		0
#define LOCK_WILL_SET		1
#define LOCK_IS_SET			2
#define LOCK_WILL_NOT_SET	3

/* ------------------------------------------------------------------------- */
/* -----------------------------    variable  global ----------------------------- */
/* ------------------------------------------------------------------------- */

// for KEY_BEYOND_FN;
extern bool isBeyondFN(void);//KEY_BEYOND_FN state

// 키를 누르거나 땔때 FN 및 LED등 을 컨트롤한다.
extern bool applyFN(uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, bool xIsDown);

extern bool isKeyEnabled(uint8_t xKeyidx);
extern void setKeyEnabled(uint8_t xKeyidx, bool xIsDown);

extern void enterFrameForFnControl(void);

#endif
