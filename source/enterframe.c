
#include "quickmacro.h"
#include "quickswap.h"
#include "custommacro.h"
#include "ledrender.h"
#include "fncontrol.h"
#include "dualaction.h"

void enterFrame(void){	

	enterFrameForMapper();
	enterFrameForQuickSwap();
	enterFrameForCustomMacro();
	enterFrameForFnControl();
	enterFrameForDualAction();

	renderLED();
	
}
