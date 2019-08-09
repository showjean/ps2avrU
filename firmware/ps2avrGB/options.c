#include <stdio.h>
#include <stdbool.h>

#include "options.h"
#include "optionsled.h"
#include "esctilde.h"
#include "fncontrol.h"
#include "hardwareinfo.h"
#include "numlocklayer.h"
#include "keymatrix.h"

void setOptions(uint8_t *data)
{
    setLedOptions(data);

    if(*(data+1) == OPTION_INDEX_ESC_TO_TILDE)
    {
        setEscTilde(*(data+2));
    }
    else if(*(data+1) == OPTION_INDEX_NUMLOCK_LAYER)
    {
        setNumlockLayer(*(data+2));
    }
    else if(*(data+1) == OPTION_INDEX_DEBOUNCE_VALUE)
    {
        setDebounceValue(*(data+2));
    }
    else if(*(data+1) == OPTION_INDEX_LED_OFF_DEFAULT)
    {
        setLedOffEnable(*(data+2));
    }
}

void getOptions(option_info_t *buffer)
{
    getLedOptions(buffer);

    // ver 1.5
    // defalut off all = 0xFF, 0=on, 1=off
    bool ledOffOption = getToggleOption(EEPROM_ENABLED_OPTION, TOGGLE_LED_OFF_DEFAULT); // true/false 주의, true 일 때 led 가 꺼진 상태;
    buffer->enableoption = ~(
            0x00 | ((isEscTilde() ? 1: 0) << TOGGLE_ESC_TO_TILDE)
             | ((ledOffOption ? 1: 0) << TOGGLE_LED_OFF_DEFAULT)
        ); 

    // Ver 1.2
    buffer->version[0] = VERSION_MAJOR;
    buffer->version[1] = VERSION_MINOR;
    buffer->version[2] = VERSION_PATCH;
    buffer->firmware = FIRMWARE;
    buffer->numlocklayer = getNumlockLayer();

    // Ver 1.3
    buffer->debouncevalue = getDebounceValue();
    buffer->size = sizeof(option_info_t);
}

