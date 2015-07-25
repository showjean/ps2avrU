del bin\*.hex

make clean
make GKP=1
copy /Y ps2avrU.hex bin\keymain_GKP.hex
::pause;
make clean
make SLA=1
copy /Y ps2avrU.hex bin\keymain_NKRO_SL_apart.hex
::pause;
make clean
make GKP=1 SLA=1
copy /Y ps2avrU.hex bin\keymain_GKP_SL_apart.hex
::pause;
make clean
make SPLIT=1
copy /Y ps2avrU.hex bin\keymain_split_NKRO.hex
::pause;
make clean
make SPLIT=1 GKP=1
copy /Y ps2avrU.hex bin\keymain_split_GKP.hex
::pause;
make clean
make
copy /Y ps2avrU.hex bin\keymain_NKRO.hex
pause;
