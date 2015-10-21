#include <stdio.h>
#include <stdbool.h>

#include "options.h"
#include "optionsled.h"
#include "esctilde.h"
#include "fncontrol.h"

void setOptions(uint8_t *data)
{
    setLedOptions(data);

    if(*(data+1) == LED2_INDEX_ESC_TO_TILDE)
    {
        setEscTilde(*(data+2));
    }
    else if(*(data+1) == LED2_INDEX_FN_LED)
    {
        setBeyondFnLed(*(data+2));
    }
}

void getOptions(led2_info_t *buffer)
{
    getLedOptions(buffer);

    buffer->esctotilde = isEscTilde();
    buffer->fnled = getBeyondFnLed();
}

void stopPwmForUsbReport(bool xIsStop)
{
    stopPwmLed(xIsStop);
}
