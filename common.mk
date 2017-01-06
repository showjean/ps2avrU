# Makefile for AVR function library development and examples
# Author: Pascal Stang
#
# For those who have never heard of makefiles: a makefile is essentially a
# script for compiling your code.  Most C/C++ compilers in the world are
# command line programs and this is even true of programming environments
# which appear to be windows-based (like Microsoft Visual C++).  Although
# you could use AVR-GCC directly from the command line and try to remember
# the compiler options each time, using a makefile keeps you free of this
# tedious task and automates the process.
#
# For those just starting with AVR-GCC and not used to using makefiles,
# I've added some extra comments above several of the makefile fields which
# you will have to deal with.

	AVRLIB = $(TOP_DIR)/avrlib
	COMMON_DIR = $(TOP_DIR)/source


#put the name of the target file here (without extension)
#  Your "target" file is your C source file that is at the top level of your code.
#  In other words, this is the file which contains your main() function.

ifndef TRG
	TRG = main
endif
	

# debug

ifdef DEBUG_LEVEL
    OPT_DEFS += -DDEBUG_LEVEL=$(DEBUG_LEVEL)
	SRC +=  $(COMMON_DIR)/usbdrv/oddebug.c
endif

ifdef DEBUG_JTAG
	OPT_DEFS += -DDEBUG_JTAG=$(DEBUG_JTAG)
endif

# Option
ifdef SCROLL_LOCK_LED_IS_APART
    OPT_DEFS += -DSCROLL_LOCK_LED_IS_APART
endif

ifdef GHOST_KEY_PREVENTION
    OPT_DEFS += -DGHOST_KEY_PREVENTION
endif

ifdef DISABLE_HARDWARE_KEYMAPPING
    OPT_DEFS += -DDISABLE_HARDWARE_KEYMAPPING
endif

ifdef DISABLE_HARDWARE_MENU
    OPT_DEFS += -DDISABLE_HARDWARE_MENU
endif

ifdef INTERFACE_ONLY_USB
    OPT_DEFS += -DINTERFACE_ONLY_USB
else
    SRC +=  $(COMMON_DIR)/keysta.c \
            $(COMMON_DIR)/ps2main.c
endif

ifdef USING_SIMPLE_MODE
	OPT_DEFS += -DUSING_SIMPLE_MODE
endif

ifdef PWM_SPEED_3
	OPT_DEFS += -DPWM_SPEED_3
endif

ifdef I2C_FULLDUPLEX
	OPT_DEFS += -DI2C_FULLDUPLEX
endif

ifdef SPLIT
	OPT_DEFS += -DSPLIT
endif

#put your C sourcefiles here 
#  Here you must list any C source files which are used by your target file.
#  They will be compiled in the order you list them, so it's probably best
#  to list $(TRG).c, your top-level target file, last.

	SRC +=  $(COMMON_DIR)/keymatrix.c \
			$(COMMON_DIR)/custommacro.c \
			$(COMMON_DIR)/keyscan.c \
			$(COMMON_DIR)/quickswap.c \
			$(COMMON_DIR)/dualaction.c \
			$(COMMON_DIR)/keydownbuffer.c \
			$(COMMON_DIR)/keyindex.c \
			$(COMMON_DIR)/keymap.c \
			$(COMMON_DIR)/sleep.c \
			$(COMMON_DIR)/quickmacro.c \
			$(COMMON_DIR)/ps2avru_util.c \
			$(COMMON_DIR)/macrobuffer.c \
			$(COMMON_DIR)/bootmapper.c \
			$(COMMON_DIR)/enterframe.c \
			$(COMMON_DIR)/fncontrol.c \
			$(COMMON_DIR)/esctilde.c \
			$(COMMON_DIR)/usbmain.c \
			$(COMMON_DIR)/usbdrv/usbdrv.c \
			$(COMMON_DIR)/timer.c \
			$(COMMON_DIR)/numlocklayer.c \
			$(COMMON_DIR)/main.c \
			
#put additional assembler source file here
#  The ASRC line allows you to list files which contain assembly code/routines that
#  you would like to use from within your C programs.  The assembly code must be
#  written in a special way to be usable as a function from your C code.

	ASRC = $(COMMON_DIR)/usbdrv/usbdrvasm.s

#additional libraries and object files to link
#  Libraries and object files are collections of functions which have already been
#  compiled.  If you have such files, list them here, and you will be able to use
#  use the functions they contain in your target program.

	LIB	=

#additional includes to compile
	INC	= $(COMMON_DIR)/usbdrv

#assembler flags
	ASFLAGS = -Wa, -gstabs -DF_CPU=$(F_CPU) -I$(COMMON_DIR)

#compiler flags
	CPFLAGS	= -g -Os -Wall -Wstrict-prototypes -I$(COMMON_DIR) -I$(INC) -I$(AVRLIB) $(LIBSRC) -DF_CPU=$(F_CPU)UL $(OPT_DEFS) -Wa,-ahlms=$(<:.c=.lst)
	
# --------------------
	CPFLAGS += -fno-inline-small-functions
	CPFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
	CPFLAGS += --param inline-call-cost=2 -finline-limit=3 
	CPFLAGS += -ffunction-sections -fdata-sections
	CPFLAGS += -Wl,--relax,--gc-sections
#	CPFLAGS += -fno-tree-scev-cprop # 최근 아래 -fno-split-wide-types 옵션과 같은 증상이 발생됨
#	CPFLAGS += -fno-split-wide-types  #usb_only 옵션으로 실행시 부트맵퍼 트리거 이상 현상 발생 (입력 row의 배수도 동시 입력) 
	#
#	CPFLAGS += -ffreestanding 
# --------------------

#linker flags
	LDFLAGS += -Wl,-Map=$(TRG).map,--cref -Wl,-u,vfprintf -lprintf_min
#	LDFLAGS = -Wl,-Map=$(TRG).map,--cref -lm
# 	KEYMAP_ADDRESS = 0x6500
# 	LDFLAGS += -Wl,--section-start=.key_matrix_basic=$(KEYMAP_ADDRESS)

	
########### you should not need to change the following line #############
include $(COMMON_DIR)/avrproj.mk


