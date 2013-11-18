make clean
make -f makefile.a87
del keymain_(a87_V03)(a87U)(mxminiU)(a87UEX).hex
ren keymain.hex keymain_(a87_V03)(a87U)(mxminiU)(a87UEX).hex
::pause;
make clean
make -f makefile.thumb
del keymain_(thumb)(face).hex
ren keymain.hex keymain_(thumb)(face).hex
::pause;
make clean
make -f makefile.bootmapper
del keymain_(bootmapper).hex
ren keymain.hex keymain_(bootmapper).hex
make clean
make -f makefile_ghost.bootmapper
del keymain_(bootmapper_ghost).hex
ren keymain.hex keymain_(bootmapper_ghost).hex
make clean
make -f makefile.mxmini
del keymain_(mxmini).hex
ren keymain.hex keymain_(mxmini).hex
pause;
