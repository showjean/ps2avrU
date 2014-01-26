
#include "keymapper.h"
#include "quickswap.h"
#include "custommacro.h"
#include "ledrender.h"
#include "fncontrol.h"

void enterFrame(void){	
	
	enterFrameForMapper();	
	enterFrameForQuickSwap();
	enterFrameForCustomMacro();
	enterFrameForFnControl();

	renderLED();
	
}
