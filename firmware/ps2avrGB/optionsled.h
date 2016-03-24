#ifndef OPTIONSLED_H_
#define OPTIONSLED_H_

#include "options.h"

void setLedOptions(uint8_t *data);

void getLedOptions(option_info_t *buffer);

void stopPwmLed(bool xIsStop);

#endif /* OPTIONSLED_H_ */
