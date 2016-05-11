
del bin\*.hex

make clean
make SPLIT=1
copy /Y ps2avrGB4U.hex bin\ps2avrGB4U_split_NKRO.hex
::pause;
make clean
make SPLIT=1 GKP=1
move /Y ps2avrGB4U.hex bin\ps2avrGB4U_split_GKP.hex
::pause;

make clean
make
copy /Y ps2avrGB4U.hex bin\ps2avrGB4U_NKRO.hex
::pause;
make clean
make GKP=1
move /Y ps2avrGB4U.hex bin\ps2avrGB4U_GKP.hex