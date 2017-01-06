
#include "esctilde.h"
#include "keydownbuffer.h"
#include "oddebug.h"

//
static bool _isEscTilde;

static bool _isEscTildeDown = false;
uint8_t getEscToTilde(uint8_t xKeyidx, bool xIsDown){
    if(xKeyidx == KEY_ESC && isEscTilde()){
    	if(xIsDown){
			uint8_t gModi = getModifierDownBuffer();
//			DBG1(0x33, (uchar *)&gModi, 1);
			if(gModi == 0x02 || gModi == 0x20){
				xKeyidx = KEY_HASH;
				_isEscTildeDown = true;
			}
    	}else{
    		if(_isEscTildeDown){
    			_isEscTildeDown = false;
    			xKeyidx = KEY_HASH;
    		}
    	}
    }
    return xKeyidx;
}

void initEscTilde(void){
	_isEscTilde = getToggleOption(EEPROM_ENABLED_OPTION, TOGGLE_ESC_TO_TILDE);

}

bool isEscTilde(void){
	return _isEscTilde;
}

void setEscTilde(bool xEnabled)
{
    _isEscTilde = xEnabled;
    setToggleOption(EEPROM_ENABLED_OPTION, TOGGLE_ESC_TO_TILDE, _isEscTilde);
}

