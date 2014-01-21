#ifndef ENTERFRAME_C
#define ENTERFRAME_C

#include "keymapper.h"
#include "quickswap.h"
#include "custommacro.h"
#include "ledrender.h"

void enterFrame(void){	
	
	enterFrameForMapper();	
	enterFrameForQuickSwap();
	enterFrameForCustomMacro();

	renderLED();
	
}

#endif