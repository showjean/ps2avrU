
#include "keymapper.h"
#include "quickswap.h"
#include "custommacro.h"
#include "ledrender.h"
#include "fncontrol.h"
#include "dualaction.h"

void enterFrame(void){	

#ifndef DISABLE_HARDWARE_MENU
	enterFrameForMapper();	
#endif
	enterFrameForQuickSwap();
	enterFrameForCustomMacro();
	enterFrameForFnControl();
	enterFrameForDualAction();

	renderLED();
	
}
