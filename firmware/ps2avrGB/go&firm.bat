..\..\..\BootMapperClient\BootMapperClient\bin\gbmgr -bootloader

make clean
::pause;
make
::make rename
bootloadHID.exe -r ps2avrGB.hex
pause;
