#ifndef KEYMAP_C
#define KEYMAP_C

#include "keymap.h"
#include "print.h"
#include "macrobuffer.h"
#include "keymapper.h"
#include "keymatrix.h"
#include "quickswap.h"

#if !(KEYMAP_A87||KEYMAP_THUMB||KEYMAP_MX_MINI||KEYMAP_BOOT_MAPPER)
	#define KEYMAP_A87
#endif


const uint8_t PROGMEM dualActionMaskDown[] = {
    KEY_FN, // FN
    KEY_FN, // FN
    KEY_FN2, // FN2
    KEY_FN2, // FN2
    KEY_RSHIFT,
    KEY_RSHIFT,    
    KEY_LSHIFT,
    KEY_LSHIFT,
    KEY_RALT,
    KEY_RALT,
    KEY_LALT,
    KEY_LALT,
    KEY_RCTRL,
    KEY_RCTRL,
    KEY_LCTRL,
    KEY_LCTRL,
    KEY_FN,
    KEY_FN2
};
const uint8_t PROGMEM dualActionMaskUp[] = {
    KEY_HANGLE, // hangle
    KEY_HANJA,  // hanja
    KEY_HANGLE, // hangle
    KEY_HANJA,  // hanja
    KEY_HANGLE,
    KEY_HANJA,  // hanja    
    KEY_HANGLE,
    KEY_HANJA,
    KEY_HANGLE,
    KEY_HANJA,
    KEY_HANGLE,
    KEY_HANJA,
    KEY_HANGLE,
    KEY_HANJA,
    KEY_HANGLE,
    KEY_HANJA,
    KEY_CAPS,
    KEY_CAPS
};
// 듀얼액션 취소시 출력할 코드값이 "다운"쪽 설정 값인지 결정;
const bool PROGMEM dualActionCancelDefaultDown[] = {
    true, 
    true,  
    true, 
    true, 
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    false,
    false
};

static uint8_t dualActionKeyIndex = 0;
static uint8_t _dualActionCount = 0;
static uint8_t _normalKeyCount = 0;
static bool _isCanceledDualAction = false;
static bool _isActiveDualAction = false;

static uint8_t getDualActionMaskUp(uint8_t keyidx);

bool isCanceledDualAction(void)
{
	return _isCanceledDualAction;
}

void applyDualActionDownWhenIsCancel(uint8_t (*func)(uint8_t, uint8_t), uint8_t isDown){
	 if(dualActionKeyIndex > 0 && isCanceledDualAction()){
        // 듀얼액션 활성화 후 다른 키가 눌려 취소되었을 때 우선 듀얼액션키의 down 값을 버퍼에 저장한다.
        (*func)(getDualActionMaskDown(dualActionKeyIndex), isDown);

        if(isMacroInput()){
        	// DEBUG_PRINT(("applyDualActionDown isMacroInput : %d\n", dualActionKeyIndex)); 
        	putKeyindex(getDualActionMaskDown(dualActionKeyIndex), 0, 0, 1);
        }
        dualActionKeyIndex = 0;
    }
}

static void applyDualActionUp(void){

	// DEBUG_PRINT(("applyDualAction \n"));

    if(dualActionKeyIndex > 0 && !isCanceledDualAction()){
        // 듀얼액션이 저장되어 있을 때 아무키도 눌리지 않은 리포트가 간다면 액션!
       	uint8_t gUpIdx = pgm_read_byte(&dualActionMaskUp[dualActionKeyIndex - (KEY_dualAction + 1)]);
        pushM(gUpIdx);
        pushM(gUpIdx);
        dualActionKeyIndex = 0;
        // DEBUG_PRINT(("applyDualActionUp : %d\n", gUpIdx)); 

        if(isMacroInput()){
        	// DEBUG_PRINT(("applyDualActionUp isMacroInput : %d\n", gUpIdx)); 
        	putKeyindex(gUpIdx, 0, 0, 1);
        	putKeyindex(gUpIdx, 0, 0, 0);
        }
    }
    // 듀얼액션 키가 모두 up 되었을 때만 active 해제;    
	if(_dualActionCount == 0) {
        _isActiveDualAction = false;

        // 모든 듀얼액션 키가 업 되었더라도 일반키는 눌려져 있을 수 있으므로 코멘트 처리;
        // _normalKeyCount = 0;
    }
}

void setDualAction(uint8_t keyidx, uint8_t isDown){
	if(isDown){
		if(keyidx > KEY_dualAction && keyidx < KEY_dualAction_end){
			++_dualActionCount;
		  	if(_isActiveDualAction == false && _normalKeyCount == 0){
		        dualActionKeyIndex = keyidx;
		        _isActiveDualAction = true;
				_isCanceledDualAction = false;
				// DEBUG_PRINT(("dualActionKeyIndex: %d \n", dualActionKeyIndex));
			}else{
	        	// 듀얼액션이 저장되어 있을 때 아무 키나 눌리면 액션 중지;    
				_isCanceledDualAction = true;
			}
	    }else if(dualActionKeyIndex > 0){
	        // 듀얼액션이 저장되어 있을 때 아무 키나 눌리면 액션 중지;    
	        // DEBUG_PRINT(("dualActionKeyIndex cancel : %d\n", dualActionKeyIndex)); 
	        _isCanceledDualAction = true;       // 듀얼액션을 취소 시키면 다음 듀얼액션 키 down일 때까지 계속 취소상태로 유지됨;
            ++_normalKeyCount;
        }else if(_isActiveDualAction == false){
            // 듀얼액션 키보다 다른 키들이 먼저 눌려진 경우 듀얼액션은 작동하지 않고 down 키값을 사용한다.
            ++_normalKeyCount;  // 보통 키가 눌려진 수만큼 증가;
            _isCanceledDualAction = true; 
	    }
	}else{
		if(keyidx > KEY_dualAction && keyidx < KEY_dualAction_end){	
			if(_dualActionCount > 0) --_dualActionCount;
			applyDualActionUp();
		}else{
            if(_normalKeyCount > 0) --_normalKeyCount;
        }

        if(isAllKeyRelease()){
            _dualActionCount = 0;
            dualActionKeyIndex = 0;
            _normalKeyCount = 0;
            _isCanceledDualAction = false;
            _isActiveDualAction = false;
        }
	}
    // DEBUG_PRINT(("_dualActionCount :::: %d\n", _dualActionCount)); 
}
uint8_t getDualActionDefaultKeyIndexWhenIsCancel(uint8_t xActionIndex){
    if(isCanceledDualAction()){
        bool gIsDefaultDown = pgm_read_byte(&dualActionCancelDefaultDown[xActionIndex - (KEY_dualAction + 1)]);
        if(gIsDefaultDown){
            return getDualActionMaskDown(xActionIndex); 
        }else{
            return getDualActionMaskUp(xActionIndex); 
        }
    }
    return xActionIndex;
}

// 듀얼액션 취소되었을 때는 다운 키코드를 적용한다.;
uint8_t getDualActionDownKeyIndexWhenIsCancel(uint8_t xActionIndex){
	if(isCanceledDualAction()){
        return getDualActionMaskDown(xActionIndex);         
    }
    return xActionIndex;
}
// 듀얼액션 키인덱스라면 다운 키코드를 반환한다.
uint8_t getDualActionMaskDown(uint8_t keyidx){
    if(keyidx > KEY_dualAction && keyidx < KEY_dualAction_end){
        keyidx = getQuickSwapKeyindex(pgm_read_byte(&dualActionMaskDown[keyidx - (KEY_dualAction + 1)])); 
    }
    return keyidx;
}
static uint8_t getDualActionMaskUp(uint8_t keyidx){
    if(keyidx > KEY_dualAction && keyidx < KEY_dualAction_end){
        keyidx = getQuickSwapKeyindex(pgm_read_byte(&dualActionMaskUp[keyidx - (KEY_dualAction + 1)])); 
    }
    return keyidx;
}

/* Originally used as a mask for the modifier bits, but now also
   used for other x -> 2^x conversions (lookup table). */
const unsigned short int modmask[16] = {
    0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000
};

// for ps/2 interface
const uint8_t PROGMEM keycode_set2[NUM_KEY] =	{ 
	0x00, 0x00, 0xFC, 0x00,
	0x1C, 0x32, 0x21, 0x23, 0x24, 0x2B, 0x34, 0x33, 0x43, 0x3B,	// abcd... 
	0x42, 0x4B, 0x3A, 0x31, 0x44, 0x4D, 0x15, 0x2D, 0x1B, 0x2C,	// klmn...
	0x3C, 0x2A, 0x1D, 0x22, 0x35, 0x1A, 0x16, 0x1E, 0x26, 0x25,	// uvwx...
	0x2E, 0x36, 0x3D, 0x3E, 0x46, 0x45, 0x5A, 0x76, 0x66, 0x0D,	// 5678...
	0x29, 0x4E, 0x55, 0x54, 0x5B, 0x5D, 0x5D, 0x4C, 0x52, 0x0E,	// space-=[...
	0x41, 0x49, 0x4A, 0x58, 0x05, 0x06, 0x04, 0x0C, 0x03, 0x0B,	// ,./caps...
	0x83, 0x0A, 0x01, 0x09, 0x78, 0x07, 0x7C, 0x7E, 0x7C, 0x70,	// F7F8F9F10... ps sl pause INSERT
	0x6C, 0x7D, 0x71, 0x69, 0x7A, 0x74, 0x6B, 0x72, 0x75, 0x77,	// HomePuDel...UP NL
	0x4A, 0x7C, 0x7B, 0x79, 0x5A, 0x69, 0x72, 0x7A, 0x6B, 0x73,	// K/ K* ... K4 K5
	0x74, 0x6C, 0x75, 0x7D, 0x70, 0x71, 0x61, 0x2F,			// K6 K7 ... APPS		// d:101

	0x00,		// KEY_Modifiers
	0x14, 0x12, 0x11, 0x1F, 0x14, 0x59, 0x11, 0x27, 
	0x00,		// KEY_Modifiers_end

	0x00,  //KEY_Multimedia	
	0x37, 0x3F, 0x5E, 0x48, 0x10, 0x3A, 0x38, 0x30, 
	0x28, 0x20, 0x18, 0x4D, 0x15, 0x3B, 0x34, 0x23, 0x32, 0x21, 
	0x50, 0x2B, 0x40, 0x4B, 0x1C, 0x43, 0x22, 0x1D,
	0x00,   //KEY_Multimedia_end                    

    0x00,  //KEY_extend 
    0xF1, 0xF2, 0x37, 0x0F, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 
    0x38, 0x40, 0x48, 0x50, 0x57, 0x5F, 0x6D, 0x00, 0x51, 0x13,
    0x6A, 0x64, 0x67, 0x27, 0x00, 0x00, 0x00, 0x63, 0x62, 0x5F,
    0x00, 0x00, 0x00, 0x00,
    0x00   //KEY_extend_end                   
};

// usage page 01(Generic Desktop) : KEY_POWER,  KEY_SLEEP,  KEY_WAKE
// usage page 07(keyboard) : KEY_HANJA,   KEY_HANGLE, KEY_KBD_POWER
/* usage page 0C(Consumer Devices) : KEY_WWW_SEARCH, KEY_WWW_HOME, KEY_WWW_BACK, KEY_WWW_FORWARD, KEY_WWW_STOP, KEY_WWW_REFRESH, KEY_WWW_FAVORITE,KEY_EMAIL,  
                                    KEY_NEXT_TRK, KEY_PREV_TRK, KEY_STOP, KEY_PLAY, KEY_MUTE, KEY_VOL_UP, KEY_VOL_DOWN, KEY_MEDIA,  KEY_CALC,   KEY_MYCOM */
//
// windows OS 에서 미지원 키 : KEY_SCREENSAVE,    KEY_REC,    KEY_REWIND,     KEY_MINIMIZE,   KEY_EJECT

const uint16_t PROGMEM keycode_USB_multimedia[] =	{ 
	0x0000, 0x0000, 0x0000, 0x018A, 0x0221, 0x0223, 0x0224, 0x0225,
	0x0226, 0x0227, 0x022A, 0x00B5, 0x00B6, 0x00B7, 0x00CD, 0x00E2, 0x00E9, 0x00EA, 
	0x0183, 0x0192, 0x0194, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};
// KEY_APPS까지는 keycode 순서에 맞게 나열되어 있지만, 
// KEY_extend는 배열 순서와는 다른 키코드를 갖는 usage page 07(keyboard)의 키코드를 정의.
const uint8_t PROGMEM keycode_USB_extend[] =   { 
    0x91, 0x90, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 
    0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x85, 0x86, 0x87, 0x88, 
    0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x92, 0x93, 0x94, 
    0x95, 0x96, 0x97, 0x98
};


const uint8_t PROGMEM keycode_set2_special[] = 
{ 	KEY_PRNSCR, KEY_PAUSE,
	KEY_NONE };

const uint8_t PROGMEM keycode_set2_makeonly[] = 
{ 	KEY_PAUSE, KEY_HANGLE, KEY_HANJA,
	KEY_NONE };

const uint8_t PROGMEM keycode_set2_make_break[] =
{ 	KEY_POWER, KEY_SLEEP, KEY_WAKE, KEY_KBD_POWER,
	KEY_NONE };

const uint8_t PROGMEM keycode_set2_extend[] =
{	KEY_LGUI, KEY_RCTRL, KEY_RGUI, KEY_RALT, KEY_APPS, KEY_PRNSCR,
	KEY_INSERT, KEY_HOME, KEY_PGUP, KEY_DEL, KEY_END, KEY_PGDN, 
	KEY_UP, KEY_LEFT, KEY_RIGHT, KEY_DOWN, KEY_KP_SLASH, KEY_KP_ENTER,
	KEY_POWER, KEY_SLEEP, KEY_WAKE, KEY_EMAIL, KEY_WWW_SEARCH, KEY_WWW_HOME,
	KEY_WWW_BACK, KEY_WWW_FORWARD, KEY_WWW_STOP, KEY_WWW_REFRESH, KEY_WWW_FAVORITE,
	KEY_NEXT_TRK, KEY_PREV_TRK, KEY_STOP, KEY_PLAY, KEY_MUTE, KEY_VOL_UP, 
	KEY_VOL_DOWN, KEY_MEDIA, KEY_CALC, KEY_MYCOM, KEY_SCREENSAVE, KEY_REC,
	KEY_REWIND, KEY_MINIMIZE, KEY_EJECT, KEY_KBD_POWER,
	KEY_NONE };

const uint8_t PROGMEM keycode_set2_proc_shift[] = 
{
	KEY_INSERT, KEY_DEL, KEY_HOME, KEY_END, KEY_PGUP, KEY_PGDN, KEY_LGUI, KEY_RGUI, KEY_APPS,
 	KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_KP_SLASH,
	KEY_NONE };





#endif
