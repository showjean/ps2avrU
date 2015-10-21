#ifndef OPTIONSLED_H_
#define OPTIONSLED_H_

void setLedOptions(uint8_t *data);

void getLedOptions(led2_info_t *buffer);

void stopPwmLed(bool xIsStop);

#endif /* OPTIONSLED_H_ */
