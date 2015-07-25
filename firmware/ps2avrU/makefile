

# Directory common source filess exist
TOP_DIR = ../..
TIMER = timer

TRG = ps2avrU

#DEBUG_LEVEL = 1

########### change this lines according to your project ##################
#put the name of the target mcu here (at90s8515, at90s8535, attiny22, atmega603 etc.)
MCU = atmega32
#	MCU = atmega161
#	MCU = atmega128

F_CPU = 12000000

SRC = ledrender_ps2avrU.c \
	vusb.c

ifdef SPLIT
	INTERFACE_ONLY_USB = 1
	SCROLL_LOCK_LED_IS_APART = 1
	DISABLE_HARDWARE_KEYMAPPING = 1
	PWM_SPEED_3 = 1
	I2C_FULLDUPLEX = 1
	
	SRC +=  i2c/i2c.c \
		ps2avrU_split_main.c
	
	ifdef GKP
		GHOST_KEY_PREVENTION = 1
		RESULT = ps2avrU_split_GKP
	else
		RESULT = ps2avrU_split_NKRO
	endif

else
	SRC +=  ps2avrU_main.c
			
	ifdef GKP
		GHOST_KEY_PREVENTION = 1
		ifdef SLA
			SCROLL_LOCK_LED_IS_APART = 1
			RESULT = ps2avrU_GKP_SLA
		else
			RESULT = ps2avrU_GKP
		endif
	else
		ifdef SLA
			SCROLL_LOCK_LED_IS_APART = 1
			RESULT = ps2avrU_NKRO_SLA
		else
			RESULT = ps2avrU_NKRO
		endif
	endif

endif

include $(TOP_DIR)/common.mk