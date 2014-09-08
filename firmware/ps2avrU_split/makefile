

# Directory common source filess exist
TOP_DIR = ../..
TIMER = timer
P2USRC = ../ps2avrU
# 현재 폴더의 파일을 우선 경로로 설정하기위해 "-I."을 우선 지정한다. 
# 하지 않을 경우, 같은 이름의 파일(hardwareinfo.h 등)이 ps2avrU의 폴더의 것으로 이용된다.
LIBSRC = -I. -I$(P2USRC)

ifndef RESULT
	RESULT = ps2avrU_split
endif

#DEBUG_LEVEL = 1

INTERFACE_ONLY_USB = 1
SCROLL_LOCK_LED_IS_APART = 1
DISABLE_HARDWARE_KEYMAPPING = 1

########### change this lines according to your project ##################
#put the name of the target mcu here (at90s8515, at90s8535, attiny22, atmega603 etc.)
MCU = atmega32
#	MCU = atmega161
#	MCU = atmega128

F_CPU = 12000000


SRC = ledrender_split.c \
	i2c/i2c.c \
	keymatrix_split.c \
	$(P2USRC)/vusb.c \
	ps2avrU_split.c

include $(TOP_DIR)/common.mk