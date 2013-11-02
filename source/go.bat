make clean
::pause;
make -f makefile.a87
pause;
::avrdude -c stk500v2 -P com3 -p atmega32 -U flash:w:thumb.hex:i
::pause;
