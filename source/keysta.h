
#ifndef KEYSTA_H

#define KEYSTA_H
#include "common_inc.h"
#include "hardwareconfig.h"
#include <stdbool.h>

/* Copyright Jamie Honan, 2001.  Distributed under the GPL.
   This program comes with ABSOLUTELY NO WARRANTY.
   See the file COPYING for license details */


/* Interface to the keyboard simulator interrupt routines is via
   flags byte and three routines. Interface is done this
   way to conserve data and program space */

/* initialise kbd isr routine */
/*#define LED_NUM_PORT		PORTD
#define LED_NUM_DDR			DDRD
#define LED_NUM_PIN			0

#define LED_CAP_PORT		PORTD
#define LED_CAP_DDR			DDRD
#define LED_CAP_PIN			1

#define LED_SCR_PORT		PORTD
#define LED_SCR_DDR			DDRD
#define LED_SCR_PIN			4*/

// #define KEYB_CLOCK_PORT		PORTD
// #define KEYB_CLOCK_DDR		DDRD
// #define KEYB_CLOCK_PINS		PIND
// #define KEYB_CLOCK_PIN		2	/* for cbi / sbi instructions */
// #define KEYB_CLOCK_BIT		0x04	/* for and/or ops */

// #define KEYB_DATA_PORT		PORTD
// #define KEYB_DATA_DDR		DDRD
// #define KEYB_DATA_PINS		PIND
// #define KEYB_DATA_PIN      3
// #define KEYB_DATA_BIT      0x08

#define KEYB_CLOCK_PORT    P2U_PS2_PORT
#define KEYB_CLOCK_DDR     P2U_PS2_DDR
#define KEYB_CLOCK_PINS    P2U_PS2_PINS
#define KEYB_CLOCK_PIN     P2U_PS2_CLOCK_PIN  /* for cbi / sbi instructions */
#define KEYB_CLOCK_BIT     P2U_PS2_CLOCK_BIT  /* for and/or ops */

#define KEYB_DATA_PORT     P2U_PS2_PORT
#define KEYB_DATA_DDR      P2U_PS2_DDR
#define KEYB_DATA_PINS     P2U_PS2_PINS
#define KEYB_DATA_PIN      P2U_PS2_DATA_PIN
#define KEYB_DATA_BIT      P2U_PS2_DATA_BIT
   
void
kbd_init(void);

/* request to transmit a character when possible. Care should be
   taken to not call this routine in the middle of already transmitting
   a character.

   Basically if kbd_flags has FLA_TX_BYTE set, then you shouldn't
   call this routine.
   */
void
kbd_set_tx(unsigned char txchar);

/* this routine gets the recently received character. How do you
   know you have a character? Because kbd_flags has FLA_RX_BYTE set.
   It could also have FLA_RX_BAD set, which means a bad character
   was recieved (e.g. bad parity).
   This routine resets the FLA_RX_BYTE flag. */

unsigned char kbd_get_rx_char(void);

bool isReadyForTx(void);
bool isReadyForRx(void);

#define FLA_CLOCK_HIGH  1
#define FLA_RX_BAD      2
#define FLA_RX_BYTE     4
#define FLA_TX_BYTE     8
#define FLA_TX_OK       0x10

#define FLA_TX_ERR	0x20

#ifdef KEYBD_EXTERN
extern volatile unsigned char	kbd_flags;
extern volatile unsigned char	kbd_state;
#endif

#endif
