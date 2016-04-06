/* Copyright Jamie Honan, 2001.  Distributed under the GPL.
   This program comes with ABSOLUTELY NO WARRANTY.
   See the file COPYING for license details. */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "global.h"
#include "timerinclude.h"
#include "common_inc.h"
#include "keysta.h"

/* This is the ISR which handles the keyboard communication
   between the host and the keyboard emulator. The interface
   routines and flag bits are commented in keysta.h

   This routine must drive and read the clock and data signals.

   Change the following few lines if you are driving your processor
   with a different crystal to 4 Mhz. Also if you wich to use different
   pins to those given here.

   Note that care should be take with oither routines that
   set DDR registers, or pin out registers that share the ports
   that are used here.

   Basically, you should do an in then out. In certain circumstances
   this read - modify - write cycle should be protected by disabling interrupts.

   Also, there is a facility for disabling the keyboard emulator
   unless a pin is held down (see DISDEBUG ifdefs below). This allows
   your keyboard cable to be pass through until you tie this pin down */

#define COUNT_UP    (256 - ((33 * CYCLES_PER_US)/8))

static void timerAction(void);

static void clockHigh(void)
{
    cbi(KEYB_CLOCK_DDR, KEYB_CLOCK_PIN); // set as input
    sbi(KEYB_CLOCK_PORT, KEYB_CLOCK_PIN); // set pullup
}

static void clockLow(void)
{
    cbi(KEYB_CLOCK_PORT, KEYB_CLOCK_PIN); // zero output value
    sbi(KEYB_CLOCK_DDR, KEYB_CLOCK_PIN); // set as output
}

static void dataHigh(void)
{
    cbi(KEYB_DATA_DDR, KEYB_DATA_PIN); // set as input
    sbi(KEYB_DATA_PORT, KEYB_DATA_PIN); // set pullup
}

static void dataLow(void)
{
    cbi(KEYB_DATA_PORT, KEYB_DATA_PIN); // zero output value
    sbi(KEYB_DATA_DDR, KEYB_DATA_PIN); // set as output
}

#define readClockPin() (KEYB_CLOCK_PINS & KEYB_CLOCK_BIT)
#define readDataPin() (KEYB_DATA_PINS & KEYB_DATA_BIT)

volatile unsigned char  kbd_flags;

enum enum_state { IDLE_START = 0, IDLE_WAIT_REL, 
    IDLE_OK_TO_TX, IDLE_END,

    RX_START = IDLE_END+10, RX_RELCLK, RX_DATA0,
    RX_DATA1, RX_DATA2, RX_DATA3, RX_DATA4,
    RX_DATA5, RX_DATA6, RX_DATA7,
    RX_PARITY, RX_STOP, RX_SENT_ACK, RX_END,

    TX_START = RX_END+10, 
    TX_DATA1, TX_DATA2, TX_DATA3, TX_DATA4,
    TX_DATA5, TX_DATA6, TX_DATA7,
    TX_PARITY, TX_STOP, TX_AFTER_STOP, TX_END } ;

volatile unsigned char kbd_state;

static volatile unsigned char   rx_byte;
static volatile unsigned char   tx_byte;
static volatile unsigned char   tx_shift;
static volatile unsigned char   parity;

void
kbd_init(void)
{
    kbd_state = IDLE_START;
    kbd_flags = FLA_CLOCK_HIGH | FLA_TX_OK;

    clockHigh();
    dataHigh();

    timer0SetPrescaler(TIMER_CLK_DIV8);
    timerAttach(TIMER0OVERFLOW_INT, timerAction);
    outp(COUNT_UP, TCNT0);  /* value counts up from this to zero */
    sbi(TIMSK, TOIE0);                      // enable TCNT0 overflow interrupt

}

    void
kbd_set_tx(unsigned char txchar)
{
    tx_byte = txchar;
    cli();
    kbd_flags &= ~FLA_TX_OK;
    kbd_flags |= FLA_TX_BYTE;
    sei();
}

    unsigned char 
kbd_get_rx_char(void)
{
    cli();
    kbd_flags &= ~FLA_RX_BYTE;
    sei();
    return rx_byte;
}

bool isReadyForRx(void)
{
    return (kbd_flags & FLA_RX_BYTE);
}

bool isReadyForTx(void)
{
    return (kbd_flags & FLA_TX_OK);
}

static void timerAction(void)
{
    /* restart timer */
    outp(COUNT_UP, TCNT0);  /* value counts up from this to zero */
    

    if (kbd_state < IDLE_END) { // start, wait_rel or ready to tx
        dataHigh();
        if (!(kbd_flags & FLA_CLOCK_HIGH)) {
            kbd_flags |= FLA_CLOCK_HIGH;
            clockHigh();
            return;
        }
        /* if clock held low, then we must prepare to start rxing */
        if (!readClockPin()) {
            kbd_state = IDLE_WAIT_REL;
            return;
        }
        switch(kbd_state) {
            case IDLE_START:
                kbd_state = IDLE_OK_TO_TX;
                return;
            case IDLE_WAIT_REL:
                if (!readDataPin()) {
                    /* PC wants to transmit */
                    kbd_state = RX_START;
                    return;
                }
                /* just an ack or something */
                kbd_state = IDLE_OK_TO_TX;
                return;
            case IDLE_OK_TO_TX:
                if (kbd_flags & FLA_TX_BYTE) {
                    dataLow();
                    kbd_state = TX_START;
                }
                return;
        }
        return;
    } else // end < IDLE_END
        if (kbd_state < RX_END) {
            if (!(kbd_flags & FLA_CLOCK_HIGH)) {
                kbd_flags |= FLA_CLOCK_HIGH;
                clockHigh();
                return;
            }
            /* at this point clock is high in preparation to going low */
            if (!readClockPin()) {
                /* PC is still holding clock down */
                dataHigh();
                kbd_state = IDLE_WAIT_REL;
                return;
            }
            switch(kbd_state) {
                case RX_START:
                    /* PC has released clock line */
                    /* we keep it high for a good half cycle */
                    kbd_flags &= ~FLA_RX_BAD;
                    kbd_state++;
                    return;
                case RX_RELCLK:
                    /* now PC has seen clock high, show it some low */
                    break;
                case RX_DATA0:
                    kbd_flags &= ~FLA_RX_BYTE;
                    if (readDataPin()) {
                        rx_byte = 0x80;
                        parity = 1;
                    } else {
                        parity = 0;
                        rx_byte = 0;
                    }
                    break; /* end clk hi 1 */
                case RX_DATA1: 
                case RX_DATA2: 
                case RX_DATA3: 
                case RX_DATA4: 
                case RX_DATA5: 
                case RX_DATA6: 
                case RX_DATA7: 
                    rx_byte >>= 1;
                    if (readDataPin()) {
                        rx_byte |= 0x80;
                        parity++;
                    }
                    break; /* end clk hi 2 to 8 */
                case RX_PARITY: 
                    if (readDataPin()) {
                        parity++;
                    }
                    if (!(parity & 1)) {
                        /* faulty, not odd parity */
                        kbd_flags |= FLA_RX_BAD;
                    }
                    break; /* end clk hi 9 */
                case RX_STOP: 
                    if (!readDataPin()) {
                        /* if stop bit not seen */
                        kbd_flags |= FLA_RX_BAD;
                    }
                    if (!(kbd_flags & FLA_RX_BAD)) {
                        dataLow();
                        kbd_flags |= FLA_RX_BYTE;
                    }
                    break; /* end clk hi 10 */
                case RX_SENT_ACK: 
                    dataHigh();
                    kbd_state = IDLE_START;
                    /* remains in clk hi 11 */
                    return;
            }
            clockLow();
            kbd_flags &= ~(FLA_CLOCK_HIGH);
            kbd_state++;
            return;
        } else // end < RX_END
            if (kbd_state < TX_END) {
                if (kbd_flags & FLA_CLOCK_HIGH) {
                    if (!readClockPin()) {
                        /* PC is still holding clock down */
                        dataHigh();
                        kbd_state = IDLE_WAIT_REL;
                        return;
                    }
                    kbd_flags &= ~FLA_CLOCK_HIGH;
                    clockLow();
                    return;
                }

                /* at this point clock is low in preparation to going high */
                kbd_flags |= FLA_CLOCK_HIGH;
                clockHigh();
                switch(kbd_state) {
                    case TX_START:
                        tx_shift = tx_byte;
                        parity = 0;
                        /* no break */
                    case TX_DATA1: 
                    case TX_DATA2: 
                    case TX_DATA3: 
                    case TX_DATA4:
                    case TX_DATA5: 
                    case TX_DATA6: 
                    case TX_DATA7:
                        if (tx_shift & 1) {
                            dataHigh();
                            parity++;
                        } else {
                            dataLow();
                        }
                        tx_shift >>= 1;
                        break; /* clock hi 1 to 8 */

                    case TX_PARITY: 
                        if (parity & 1) {
                            dataLow();
                        } else {
                            dataHigh();
                        }
                        kbd_flags &= ~FLA_TX_BYTE;
                        kbd_flags |= FLA_TX_OK;
                        break; /* clock hi 9 */
                    case TX_STOP: 
                        dataHigh();
                        break; /* clock hi 10 */
                    case TX_AFTER_STOP:
                        kbd_state = IDLE_START;
                        /* remains in clk hi 11 */
                        return;
                }
                kbd_state++;

            } //else

}
