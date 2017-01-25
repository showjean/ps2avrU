#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H


/* report id */
#define REPORT_ID_INFO      1
#define REPORT_ID_BOOT      2
#define REPORT_ID_SYSTEM    4
#define REPORT_ID_CONSUMER  3
#define REPORT_ID_MOUSE     5


/* ----------------------- PS/2 ------------------------ */
#define P2U_PS2_PORT		PORTD
#define P2U_PS2_DDR			DDRD
#define P2U_PS2_PINS		PIND
#define P2U_PS2_CLOCK_PIN		2	/* for cbi / sbi instructions */
#define P2U_PS2_CLOCK_BIT		0x04	/* for and/or ops */
#define P2U_PS2_DATA_PIN      3
#define P2U_PS2_DATA_BIT      0x08

/* ----------------------- USB ------------------------ */
#define P2U_USB_CFG_DDR			    DDRD
#define P2U_USB_CFG_IOPORTNAME      D
#define P2U_USB_CFG_DMINUS_BIT      3
#define P2U_USB_CFG_DPLUS_BIT       2

#define P2U_USB_COUNT_SOF			1

#endif
