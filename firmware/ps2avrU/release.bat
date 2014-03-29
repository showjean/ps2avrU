make clean
make -f makefile_NKRO
del keymain_NKRO.hex
ren main.hex keymain_NKRO.hex
::pause;
make clean
make -f makefile_GKP
del keymain_GKP.hex
ren main.hex keymain_GKP.hex
::pause;
make clean
make -f makefile_NKRO_SL_apart
del keymain_NKRO_SL_apart.hex
ren main.hex keymain_NKRO_SL_apart.hex
::pause;
make clean
make -f makefile_GKP_SL_apart
del keymain_GKP_SL_apart.hex
ren main.hex keymain_GKP_SL_apart.hex
::pause;
go.bat
pause;
