
#include "keymain.h"

uint8_t getBootmapperStatus(uint8_t xCol, uint8_t xRow){
	if(xCol == 1 && xRow == 1){
		return 1;
	}else{
		return 0;
	}
}
