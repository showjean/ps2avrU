make clean -f makefile.keymap_part
::pause;
make -f makefile.keymap_part
pause;
::avrdude -c stk500v2 -P com3 -p atmega32 -U flash:w:thumb.hex:i
::pause;
