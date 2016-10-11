
#include <stdio.h>
#include <stdbool.h>
#include <avr/eeprom.h>

#include "numlocklayer.h"
#include "ledrender.h"

static uint8_t _numlockLayer;

void initNumlockLayer(void)
{
    _numlockLayer = eeprom_read_byte((uint8_t *)EEPROM_NUMLOCK_LAYER);
    if(_numlockLayer == 0xFF)
    {
        _numlockLayer = 0;
    }
}

uint8_t getNumlockLayer(void)
{
    if(getLEDState() & LED_STATE_NUM)
    {
        return _numlockLayer;
    }
    else
    {
        return 0;
    }
}

void setNumlockLayer(uint8_t xNumlockLayer)
{
    _numlockLayer = xNumlockLayer;
    eeprom_update_byte((uint8_t *)EEPROM_NUMLOCK_LAYER, _numlockLayer);
}
