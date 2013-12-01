make clean -f makefile.keymap_part
make -f makefile.keymap_part_a87
del keymap_part(a87_V03)(a87U)(mxminiU)(a87UEX).hex
ren keymap_part.hex keymap_part(a87_V03)(a87U)(mxminiU)(a87UEX).hex
::pause;
make clean -f makefile.keymap_part
make -f makefile.keymap_part_thumb
del keymap_part(thumb)(face).hex
ren keymap_part.hex keymap_part(thumb)(face).hex
::pause;
make clean -f makefile.keymap_part
make -f makefile.keymap_part_bootmapper
del keymap_part(none).hex
ren keymap_part.hex keymap_part(none).hex
::pause;
make clean -f makefile.keymap_part
pause;
