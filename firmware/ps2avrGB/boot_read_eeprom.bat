avrdude -c stk500v2 -P com3 -p atmega32 -U eeprom:r:bread.hex:i
pause;
