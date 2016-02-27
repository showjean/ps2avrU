

# Directory common source filess exist
TOP_DIR = ../..
#TIMER = timer

#FLASH_WRITE_ADDRESS = 0x7E00

TRG = ps2avrGB

#DEBUG_LEVEL = 1
#DEBUG_JTAG = 1

ifdef DEBUG_JTAG
LDFLAGS += -Wl,--section-start=.keymapCodeJtag=0x6600
endif 

#INTERFACE_ONLY_USB = 1
SCROLL_LOCK_LED_IS_APART = 1
DISABLE_HARDWARE_KEYMAPPING = 1
#DISABLE_HARDWARE_MENU = 1
USING_CUSTOM_TIMER = 1
#USING_SIMPLE_MODE = 1
#SPLIT = 1


########### change this lines according to your project ##################
#put the name of the target mcu here (at90s8515, at90s8535, attiny22, atmega603 etc.)
MCU = atmega32
# MCU = atmega161
# MCU = atmega128

F_CPU = 12000000
# Light_WS2812/light_ws2812.c \

SRC =i2c/i2c.c \
	timer/timer.c \
	ledrender.c \
	options.c \
	vusb.c


ifdef SPLIT
	PWM_SPEED_3 = 1
	I2C_FULLDUPLEX = 1
	INTERFACE_ONLY_USB = 1
	
	SRC +=  ps2avrGB_split_main.c 

	ifdef GKP
		GHOST_KEY_PREVENTION = 1
		RESULT = ps2avrGB_split_GKP
	else
		RESULT = ps2avrGB_split_NKRO
	endif

else
	SRC +=  ps2avrGB_main.c
	
	ifdef GKP
		GHOST_KEY_PREVENTION = 1
		RESULT = ps2avrGB_GKP
	else
		RESULT = ps2avrGB_NKRO
	endif
endif



# boot
ifdef FLASH_WRITE_ADDRESS
LDFLAGS += -Wl,--section-start=.flashWrite=$(FLASH_WRITE_ADDRESS)
endif 

include $(TOP_DIR)/common.mk
