

# Directory common source filess exist
TOP_DIR = ../..
P2GBSRC = ../ps2avrGB
# 현재 폴더의 파일을 우선 경로로 설정하기위해 "-I."을 우선 지정한다. 
# 하지 않을 경우, 같은 이름의 파일(hardwareinfo.h 등)이 ps2avrU의 폴더의 것으로 이용된다.
LIBSRC = -I. -I$(P2GBSRC)

#FLASH_WRITE_ADDRESS = 0x7E00

TRG = ps2avrGB4U

#DEBUG_LEVEL = 1
#DEBUG_JTAG = 1

#INTERFACE_ONLY_USB = 1
SCROLL_LOCK_LED_IS_APART = 1
DISABLE_HARDWARE_KEYMAPPING = 1
#DISABLE_HARDWARE_MENU = 1
#USING_SIMPLE_MODE = 1
#SPLIT = 1


########### change this lines according to your project ##################
#put the name of the target mcu here (at90s8515, at90s8535, attiny22, atmega603 etc.)
MCU = atmega32

F_CPU = 12000000

SRC = vusb.c \
	$(P2GBSRC)/i2c/i2c.c \
	$(P2GBSRC)/ledrender.c \
	$(P2GBSRC)/options.c


ifdef SPLIT
	PWM_SPEED_3 = 1
	I2C_FULLDUPLEX = 1
	INTERFACE_ONLY_USB = 1
	
	SRC +=  ps2avrGB4U_split_main.c 

	ifdef GKP
		GHOST_KEY_PREVENTION = 1
		RESULT = ps2avrGB4U_split_GKP
	else
		RESULT = ps2avrGB4U_split_NKRO
	endif

else
	SRC +=  ps2avrGB4U_main.c
	
	ifdef GKP
		GHOST_KEY_PREVENTION = 1
		RESULT = ps2avrGB4U_GKP
	else
		RESULT = ps2avrGB4U_NKRO
	endif
endif



# boot
ifdef FLASH_WRITE_ADDRESS
LDFLAGS += -Wl,--section-start=.flashWrite=$(FLASH_WRITE_ADDRESS)
endif 

include $(TOP_DIR)/common.mk
