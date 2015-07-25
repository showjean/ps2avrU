::make clean
::make -f makefile_NKRO.mk
::del ps2avrGB_NKRO.hex
::copy ps2avrGB.hex ps2avrGB_NKRO.hex
::pause;

del bin\*.hex

make clean
make SPLIT=1
copy /Y ps2avrGB.hex bin\ps2avrGB_split_NKRO.hex
::pause;
::make clean
::make SPLIT=1 GKP=1
::move /Y ps2avrGB.hex bin\ps2avrGB_split_GKP.hex
::pause;

make clean
make
copy /Y ps2avrGB.hex bin\ps2avrGB_NKRO.hex
::pause;
::make clean
::make GKP=1
::move /Y ps2avrGB.hex bin\ps2avrGB_GKP.hex