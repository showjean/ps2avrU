#ifndef ENTERFRAME_C
#define ENTERFRAME_C

#include "keymapper.h"
#include "quickswap.h"
#include "custommacro.h"

void enterFrame(void){	
	
	enterFrameForMapper();	
	enterFrameForQuickSwap();
	enterFrameForCustomMacro();
	
}

#endif