make clean
make -f makefile_NKRO
del keymain_split_NKRO.hex
ren main.hex keymain_split_NKRO.hex
::pause;
make clean
make -f makefile_GKP
del keymain_split_GKP.hex
ren main.hex keymain_split_GKP.hex

::pause;
go.bat
pause;
