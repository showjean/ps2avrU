make clean
make -f makefile_NKRO.mk
del ps2avrGB_NKRO.hex
ren main.hex ps2avrGB_NKRO.hex
::pause;
::make clean
::make -f makefile_GKP.mk
::del ps2avrGB_GKP.hex
::ren main.hex ps2avrGB_GKP.hex
::pause;
go.bat
pause;
