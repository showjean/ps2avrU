/*
Copyright 2011 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "usbdrv/usbdrv.h"
#include "usbconfig.h"
#include "vusb.h"

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

// report_keyboard_t reportKeyboard;
uint8_t reportKeyboard[REPORT_SIZE_KEYBOARD];
uint8_t idleRate = 0;        ///< in 4ms units
static uint8_t protocolVer = 1; ///< 0 = boot protocol, 1 = report protocol
static uint8_t expectReport = 0;       ///< flag to indicate if we expect an USB-report


/*------------------------------------------------------------------*
 * Descriptors                                                      *
 *------------------------------------------------------------------*/

/*
 * Report Descriptor for keyboard
 *
 * from an example in HID spec appendix
 */
PROGMEM const uchar keyboard_hid_report[] = {
    0x05, 0x01,   // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,   // USAGE (Keyboard)
    0xa1, 0x01,   // COLLECTION (Application)

    // 0x85, 0x07,         //   REPORT_ID (7)

    /* modifiers */
    0x05, 0x07,   //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,   //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,   //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,   //   LOGICAL_MINIMUM (0)
    0x25, 0x01,   //   LOGICAL_MAXIMUM (1)
    0x95, 0x08,   //   REPORT_COUNT (8)
    0x75, 0x01,   //   REPORT_SIZE (1)
    0x81, 0x02,   //   INPUT (Data,Var,Abs)
    /* modifiers end */

    0x95, 0x01,   //   REPORT_COUNT (1)
    0x75, 0x08,   //   REPORT_SIZE (8)
    0x81, 0x03,   //   INPUT (Cnst,Var,Abs) 
    
    /* leds */
    0x95, 0x05,   //   REPORT_COUNT (5)
    0x75, 0x01,   //   REPORT_SIZE (1)
    0x05, 0x08,   //   USAGE_PAGE (LEDs)
    0x19, 0x01,   //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,   //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,   //   OUTPUT (Data,Var,Abs)
    0x95, 0x01,   //   REPORT_COUNT (1)
    0x75, 0x03,   //   REPORT_SIZE (3)
    0x91, 0x03,   //   OUTPUT (Cnst,Var,Abs)
    /* leds end */

    /* keyboard body */
    0x05, 0x07,   //   USAGE_PAGE (Keyboard)
    0x19, 0x00,   //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0xFF,   //   USAGE_MAXIMUM
    0x95, 0x06,   //   REPORT_COUNT (6)
    0x75, 0x08,   //   REPORT_SIZE (8)
    0x15, 0x00,   //   LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,  //   LOGICAL_MAXIMUM (255)
    0x81, 0x00,   //   INPUT (Data,Ary,Abs)
    /* keyboard body end */

    /* consumer device */
    // 0x05, 0x0C,   // USAGE_PAGE (Consumer Devices)
    // 0x75, 0x01,   //   REPORT_SIZE (1)
    // 0x95, 0x01,   //   REPORT_COUNT (1)
    // 0x09, 0xB8,   //   USAGE (Eject)
    // 0x15, 0x00,   //   LOGICAL_MINIMUM (0)
    // 0x25, 0x01,   //   LOGICAL_MAXIMUM (1)
    // 0x81, 0x02,   //   INPUT (Data,Var,Abs)
    // 0x05, 0xFF,   // USAGE_PAGE
    // 0x09, 0x03,   //   USAGE 
    // 0x75, 0x07,   //   REPORT_SIZE (7)
    // 0x95, 0x01,   //   REPORT_COUNT (1)
    // 0x81, 0x02,   //   INPUT (Data,Var,Abs) 
    /* consumer device end */

    0xC0                           // END_COLLECTION
};

/*
 * Report Descriptor for mouse
 *
 * Mouse Protocol 1, HID 1.11 spec, Appendix B, page 59-60, with wheel extension
 * http://www.microchip.com/forums/tm.aspx?high=&m=391435&mpage=1#391521
 * http://www.keil.com/forum/15671/
 * http://www.microsoft.com/whdc/device/input/wheel.mspx
 */
PROGMEM const uchar mouse_hid_report[] = {
    /* mouse */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, REPORT_ID_MOUSE,         //   REPORT_ID (1)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
                                   // ----------------------------  Buttons
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x05,                    //     USAGE_MAXIMUM (Button 5)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x95, 0x05,                    //     REPORT_COUNT (5)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x75, 0x03,                    //     REPORT_SIZE (3)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
                                   // ----------------------------  X,Y position
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
                                   // ----------------------------  Vertical wheel
    0x09, 0x38,                    //     USAGE (Wheel)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)        - reset physical
    0x45, 0x00,                    //     PHYSICAL_MAXIMUM (0)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
                                   // ----------------------------  Horizontal wheel
    0x05, 0x0c,                    //     USAGE_PAGE (Consumer Devices)
    0x0a, 0x38, 0x02,              //     USAGE (AC Pan)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xc0,                          //   END_COLLECTION
    0xc0,                          // END_COLLECTION
    /* system control */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x80,                    // USAGE (System Control)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, REPORT_ID_SYSTEM,        //   REPORT_ID (2)
    0x15, 0x01,                    //   LOGICAL_MINIMUM (0x1)
    0x25, 0xb7,                    //   LOGICAL_MAXIMUM (0xb7)
    0x19, 0x01,                    //   USAGE_MINIMUM (0x1)
    0x29, 0xb7,                    //   USAGE_MAXIMUM (0xb7)
    0x75, 0x10,                    //   REPORT_SIZE (16)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x00,                    //   INPUT (Data,Array,Abs)
    0xc0,                          // END_COLLECTION
    /* consumer */
    0x05, 0x0c,                    // USAGE_PAGE (Consumer Devices)
    0x09, 0x01,                    // USAGE (Consumer Control)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, REPORT_ID_CONSUMER,      //   REPORT_ID (3)
    0x15, 0x01,                    //   LOGICAL_MINIMUM (0x1)
    0x26, 0x9c, 0x02,              //   LOGICAL_MAXIMUM (0x29c)
    0x19, 0x01,                    //   USAGE_MINIMUM (0x1)
    0x2a, 0x9c, 0x02,              //   USAGE_MAXIMUM (0x29c)
    0x75, 0x10,                    //   REPORT_SIZE (16)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x00,                    //   INPUT (Data,Array,Abs)
    0xc0,                          // END_COLLECTION
};


/* 
 * Descriptor for compite device: Keyboard + Mouse
 * 
 * contains: device, interface, HID and endpoint descriptors
 */
#if USB_CFG_DESCR_PROPS_CONFIGURATION
PROGMEM const char usbDescriptorConfiguration[] = {    /* USB configuration descriptor */
    9,          /* sizeof(usbDescriptorConfiguration): length of descriptor in bytes */
    USBDESCR_CONFIG,    /* descriptor type */
    9 + (9 + 9 + 7) + (9 + 9 + 7), 0,
    //18 + 7 * USB_CFG_HAVE_INTRIN_ENDPOINT + 7 * USB_CFG_HAVE_INTRIN_ENDPOINT3 + 9, 0,
                /* total length of data returned (including inlined descriptors) */
    2,          /* number of interfaces in this configuration */
    1,          /* index of this configuration */
    0,          /* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
    (1 << 7) | USBATTR_SELFPOWER,       /* attributes */
#else
    (1 << 7),                           /* attributes */
#endif
    USB_CFG_MAX_BUS_POWER/2,            /* max USB current in 2mA units */

    /*
     * Keyboard interface
     */
    /* Interface descriptor */
    9,          /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE, /* descriptor type */
    0,          /* index of this interface */
    0,          /* alternate setting for this interface */
    USB_CFG_HAVE_INTRIN_ENDPOINT, /* endpoints excl 0: number of endpoint descriptors to follow */
    USB_CFG_INTERFACE_CLASS,
    USB_CFG_INTERFACE_SUBCLASS,
    USB_CFG_INTERFACE_PROTOCOL,
    0,          /* string index for interface */
    /* HID descriptor */
    9,          /* sizeof(usbDescrHID): length of descriptor in bytes */
    USBDESCR_HID,   /* descriptor type: HID */
    0x01, 0x01, /* BCD representation of HID version */
    0x00,       /* target country code */
    0x01,       /* number of HID Report (or other HID class) Descriptor infos to follow */
    0x22,       /* descriptor type: report */
    sizeof(keyboard_hid_report), 0,  /* total length of report descriptor */
    /* Endpoint descriptor */
#if USB_CFG_HAVE_INTRIN_ENDPOINT    /* endpoint descriptor for endpoint 1 */
    7,          /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    (char)0x81, /* IN endpoint number 1 */
    0x03,       /* attrib: Interrupt endpoint */
    8, 0,       /* maximum packet size */
    USB_CFG_INTR_POLL_INTERVAL, /* in ms */
#endif

    /*
     * Mouse interface
     */
    /* Interface descriptor */
    9,          /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE, /* descriptor type */
    1,          /* index of this interface */
    0,          /* alternate setting for this interface */
    USB_CFG_HAVE_INTRIN_ENDPOINT3, /* endpoints excl 0: number of endpoint descriptors to follow */
    0x03,       /* CLASS: HID */
    0,          /* SUBCLASS: none */
    0,          /* PROTOCOL: none */
    0,          /* string index for interface */
    /* HID descriptor */
    9,          /* sizeof(usbDescrHID): length of descriptor in bytes */
    USBDESCR_HID,   /* descriptor type: HID */
    0x01, 0x01, /* BCD representation of HID version */
    0x00,       /* target country code */
    0x01,       /* number of HID Report (or other HID class) Descriptor infos to follow */
    0x22,       /* descriptor type: report */
    sizeof(mouse_hid_report), 0,  /* total length of report descriptor */
#if USB_CFG_HAVE_INTRIN_ENDPOINT3   /* endpoint descriptor for endpoint 3 */
    /* Endpoint descriptor */
    7,          /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    (char)(0x80 | USB_CFG_EP3_NUMBER), /* IN endpoint number 3 */
    0x03,       /* attrib: Interrupt endpoint */
    8, 0,       /* maximum packet size */
    USB_CFG_INTR_POLL_INTERVAL, /* in ms */
#endif
};
#endif

USB_PUBLIC usbMsgLen_t usbFunctionDescriptor(struct usbRequest *rq)
{
    usbMsgLen_t len = 0;

/*
    debug("usbFunctionDescriptor: ");
    debug_hex(rq->bmRequestType); debug(" ");
    debug_hex(rq->bRequest); debug(" ");
    debug_hex16(rq->wValue.word); debug(" ");
    debug_hex16(rq->wIndex.word); debug(" ");
    debug_hex16(rq->wLength.word); debug("\n");
*/
    switch (rq->wValue.bytes[1]) {
#if USB_CFG_DESCR_PROPS_CONFIGURATION
        case USBDESCR_CONFIG:
            usbMsgPtr = (usbMsgPtr_t)usbDescriptorConfiguration;
            len = sizeof(usbDescriptorConfiguration);
            break;
#endif
        case USBDESCR_HID:
            switch (rq->wValue.bytes[0]) {
                case 0:
                    usbMsgPtr = (usbMsgPtr_t)(usbDescriptorConfiguration + 9 + 9);
                    len = 9;
                    break;
                case 1:
                    usbMsgPtr = (usbMsgPtr_t)(usbDescriptorConfiguration + 9 + (9 + 9 + 7) + 9);
                    len = 9;
                    break;
            }
            break;
        case USBDESCR_HID_REPORT:
            /* interface index */
            switch (rq->wIndex.word) {
                case 0:
                    usbMsgPtr = (usbMsgPtr_t)keyboard_hid_report;
                    len = sizeof(keyboard_hid_report);
                    break;
                case 1:
                    usbMsgPtr = (usbMsgPtr_t)mouse_hid_report;
                    len = sizeof(mouse_hid_report);
                    break;
            }
            break;
    }
    //debug("desc len: "); debug_hex(len); debug("\n");
    return len;
}



/**
 * This function is called whenever we receive a setup request via USB.
 * \param data[8] eight bytes of data we received
 * \return number of bytes to use, or 0xff if usbFunctionWrite() should be
 * called
 */
uint8_t usbFunctionSetup(uint8_t data[8]) {

    delegateInterfaceReadyUsb(); 

    usbRequest_t *rq = (void *)data;

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        // class request type
        if (rq->bRequest == USBRQ_HID_GET_REPORT) {
            // wValue: ReportType (highbyte), ReportID (lowbyte)
            // we only have one report type, so don't look at wValue
            usbMsgPtr = (usbMsgPtr_t)&reportKeyboard;
            return sizeof(reportKeyboard);
        } else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
            // MAC OS X is not processing here
              
            // Report Type: 0x02(Out)/ReportID: 0x00(none) && Interface: 0(keyboard)
            if (rq->wValue.word == 0x0200 && rq->wIndex.word == 0) 
            {
                // We expect one byte reports
                expectReport = 1;
                return 0xff; // Call usbFunctionWrite with data
            }
        } else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
            usbMsgPtr = idleRate;
            return 1;
        } else if (rq->bRequest == USBRQ_HID_SET_IDLE) {

            delegateInitInterfaceUsb();

            idleRate = rq->wValue.bytes[1];
        } else if (rq->bRequest == USBRQ_HID_GET_PROTOCOL) {
            if (rq->wValue.bytes[1] < 1) {
                protocolVer = rq->wValue.bytes[1];
            }
        } else if(rq->bRequest == USBRQ_HID_SET_PROTOCOL) {
            usbMsgPtr = protocolVer;
            return 1;
        }
    } else {        
        // no vendor specific requests implemented
    }

    return 0;
}


/**
 * The write function is called when LEDs should be set. Normally, we get only
 * one byte that contains info about the LED states.
 * \param data pointer to received data
 * \param len number ob bytes received
 * \return 0x01
 */
 // function is not working in mac os 
uint8_t usbFunctionWrite(uchar *data, uchar len) {
    if (expectReport && (len == 1)) {
        delegateLedUsb(data[0]); 
    }
    expectReport = 0;
    return 0x01;
}

