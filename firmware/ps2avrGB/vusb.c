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
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "usbdrv/usbdrv.h"
#include "usbconfig.h"
#include "vusb.h"
#include "main.h"
#include "oddebug.h"
#include "boot.h"
#include "led2.h"
#include "ledrender.h"
#include "ledconfig.h"
#include "eeprominfo.h"
#include "bootmapper.h"
#include "timerinclude.h"

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

// report_keyboard_t reportKeyboard;
uint8_t reportKeyboard[REPORT_SIZE_KEYBOARD];
uint8_t idleRate = 0;        ///< in 4ms units
static uint8_t protocolVer = 1; ///< 0 = boot protocol, 1 = report protocol
uint8_t expectReport = 0;       ///< flag to indicate if we expect an USB-report


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
PROGMEM const uchar custom_hid_report[] = {
	/* mouse */
/*
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
*/

    /* consumer */
    0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)

    0x85, REPORT_ID_INFO,         //   REPORT_ID (1)
    0x95, 0x07,                    //   REPORT_COUNT (7)
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)

    0x85, REPORT_ID_BOOT,          //   REPORT_ID (2)
    0x95, 0x83,              		//   REPORT_COUNT (131)
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
    0xc0,                           // END_COLLECTION

    /* system control */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x80,                    // USAGE (System Control)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, REPORT_ID_SYSTEM,        //   REPORT_ID (4)
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
#if defined( USING_SIMPLE_MODE )
    1,          /* number of interfaces in this configuration */
#else
    2,          /* number of interfaces in this configuration */
#endif
    1,          /* index of this configuration */
    0,          /* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
    (1 << 7) | USBATTR_SELFPOWER,       /* attributes */
#else
    (1 << 7) | USBATTR_REMOTEWAKE,      /* attributes */
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

#if !defined( USING_SIMPLE_MODE )
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
    sizeof(custom_hid_report), 0,  /* total length of report descriptor */
#if USB_CFG_HAVE_INTRIN_ENDPOINT3   /* endpoint descriptor for endpoint 3 */
    /* Endpoint descriptor */
    7,          /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    (char)(0x80 | USB_CFG_EP3_NUMBER), /* IN endpoint number 3 */
    0x03,       /* attrib: Interrupt endpoint */
    8, 0,       /* maximum packet size */
    USB_CFG_INTR_POLL_INTERVAL, /* in ms */
#endif
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
                    usbMsgPtr = (usbMsgPtr_t)custom_hid_report;
                    len = sizeof(custom_hid_report);
                    break;
            }
            break;
    }
    //debug("desc len: "); debug_hex(len); debug("\n");
//    DBG1(0x22,0,0);
    return len;
}


#define HID_REPORT_KEBOARD  0x0200
#define HID_REPORT_BOOT     (0x0300 | REPORT_ID_BOOT)
#define HID_REPORT_OPTION   (0x0300 | REPORT_ID_INFO)

//static uchar isStart = 0;

static uint8_t readyForRainbowColor = 0;

/**
 * This function is called whenever we receive a setup request via USB.
 * \param data[8] eight bytes of data we received
 * \return number of bytes to use, or 0xff if usbFunctionWrite() should be
 * called
 */
usbMsgLen_t usbFunctionSetup(uint8_t data[8]) {

    delegateInterfaceReadyUsb();

    usbRequest_t *rq = (void *)data;
//    DBG1(0xCC, data, 8);

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        // class request type
        if (rq->bRequest == USBRQ_HID_GET_REPORT) {
            if (rq->wValue.word == HID_REPORT_KEBOARD)
            {
                // wValue: ReportType (highbyte), ReportID (lowbyte)
                usbMsgPtr = (usbMsgPtr_t)&reportKeyboard;
                return sizeof(reportKeyboard);
            }else if(rq->wValue.word == HID_REPORT_BOOT){

            	if(rq->wLength.word == LED2_GET_REPORT_LENGTH_RAINBOW){	// ready for rainbow color setting;
            		readyForRainbowColor = 1;
            	}
//                // boot
//            	static uchar    optionsBuffer[0x84];
//
//            	memset(optionsBuffer, 0, 0x83);
//            	optionsBuffer[0]       = REPORT_ID_BOOT;	// LED options
//            	/* LED options */
//            	optionsBuffer[0x83] = 0x84;
//            	usbMsgPtr = (usbMsgPtr_t)optionsBuffer;

//            	return sizeof(optionsBuffer);
            }else if(rq->wValue.word == HID_REPORT_OPTION){
            	// length : rq->wLength.word 필요한 리포트를 length로 구분한다.

        		uint8_t k, j, i = 0;
        		static uchar    led2Buffer[CUSTOM_MACRO_SIZE_MAX];
            	if(rq->wLength.word == LED2_GET_REPORT_LENGTH_INFO){
            		// report led2 info
            		getLed2((led2_info_t *)led2Buffer);
            		usbMsgPtr = (usbMsgPtr_t)led2Buffer;
            		return LED2_GET_REPORT_LENGTH_INFO; 	//sizeof(led2Buffer);
            	}else if(rq->wLength.word >= LED2_GET_REPORT_LENGTH_KEYMAP_LAYER1 && rq->wLength.word <= LED2_GET_REPORT_LENGTH_KEYMAP_LAYER4){
            		// keymap
					for(k = 0; k < ROWS; ++k){
						for (j = 0; j < COLUMNS; ++j)
						{
							led2Buffer[i++] = pgm_read_byte(KEYMAP_ADDRESS+(ROWS * COLUMNS * (rq->wLength.word - LED2_GET_REPORT_LENGTH_KEYMAP_LAYER1))+(k * COLUMNS + j));
						}
					}
//					DBG1(0x89, led2Buffer, 120);
					usbMsgPtr = (usbMsgPtr_t)led2Buffer;
					return LED2_GET_REPORT_LENGTH_KEYMAP;
            	}else if(rq->wLength.word >= LED2_GET_REPORT_LENGTH_MACRO1 && rq->wLength.word <= LED2_GET_REPORT_LENGTH_MACRO12){
            		// cst macro
            		for(k = 0; k < CUSTOM_MACRO_SIZE_MAX; ++k){
            			led2Buffer[i++] = pgm_read_byte(CUSTOM_MACRO_ADDRESS+(CUSTOM_MACRO_SIZE_MAX * (rq->wLength.word - LED2_GET_REPORT_LENGTH_MACRO1))+(k));
            		}
            		usbMsgPtr = (usbMsgPtr_t)led2Buffer;
					return CUSTOM_MACRO_SIZE_MAX;
            	}else {
            		return rq->wLength.word;
            	}
            }
        } else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
//        	DBG1(0xAA, (uchar *)&rq->wValue.word, 2);
        	// 02 03 : Report Type: 0x03,  ReportID: 0x02
        	// 01 03 : Report Type: 0x03,  ReportID: 0x01
            // Report Type: 0x02(Out)/ReportID: 0x00(none) && Interface: 0(keyboard)
            if (rq->wValue.word == HID_REPORT_KEBOARD && rq->wIndex.word == 0)
            {
                // We expect one byte reports
                expectReport = 1;
                return USB_NO_MSG; // Call usbFunctionWrite with data
            }else if(rq->wValue.word == HID_REPORT_BOOT){
                // boot
//                isStart = 1;
                if(readyForRainbowColor==1){
                	data[1] = LED2_INDEX_COLOR_RAINBOW_INIT;
                	setLed2((uint8_t *)data);
                	expectReport = 4;
                }else{
                	expectReport = 3;
                }
                readyForRainbowColor = 0;
                return USB_NO_MSG; // Call usbFunctionWrite with data
            }else if(rq->wValue.word == HID_REPORT_OPTION){
            	// options
                expectReport = 2;

                return USB_NO_MSG; // Call usbFunctionWrite with data
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


//
//uint8_t _hasBootUpdate;
//uint8_t _pageCount = 0;
//uint32_t address[BOOT_PAGE_COUNT];
//uint8_t buffer[BOOT_PAGE_COUNT][SPM_PAGESIZE];
//
//uint8_t writeFlash(uchar *data, uchar len, uchar isNew){
//
//	uint8_t isLast;
//	static uchar offset;
//	static uchar bufferIndex;
//
////    DBG1(0x3A, data, len);
//
//    if(isNew == 1){
//        offset = 0;
//        memset(buffer[_pageCount], 0, SPM_PAGESIZE);
//        DBG1(0x30, data, 3);
//        address[_pageCount] = data[1] | (data[2]<<8);
//        data += 4;
//        len -= 4;
//        bufferIndex = 0;
//    }
////    DBG1(0x31, (void *)&address, sizeof(address));
//    offset += len;
//    isLast = offset & 0x80;  //!= 0 if last block received
//    do{
//    	buffer[_pageCount][bufferIndex] = *data++;
//    	bufferIndex++;
//        len -= 1;
//    }while(len);
//
//    if(isLast != 0){
//
//    	 /* 3줄 코멘트 : 작동 됨;
//    	 * callProgramPage만 코멘트 : 작동 됨;
//    	 * callProgramPage만 실행 : 리셋됨
//    	 * 3줄 모두 실행 : 커뮤니케이션 에러;
//    	 */
//    	cli();
//    	callProgramPage(address[_pageCount], (uint8_t *)buffer);
//    	sei();
////    	DBG1(0x3F, (void *)&address[_pageCount], 2);
////    	DBG1(0x3F, buffer[_pageCount], SPM_PAGESIZE);
//        _pageCount = _pageCount + 1;
//    	if(_pageCount == BOOT_PAGE_COUNT){
//    		_hasBootUpdate = 1;
//    		_pageCount = 0;
//    	}
//    }
////    DBG1(0x35, 0, 0);
//
//    return isLast;
//}
/**
 * The write function is called when LEDs should be set. Normally, we get only
 * one byte that contains info about the LED states.
 * \param data pointer to received data
 * \param len number ob bytes received
 * \return 0x01
 */
uint8_t usbFunctionWrite(uchar *data, uchar len) {
//    DBG1(0xBB, (uchar *)&len, 1);
    if (expectReport == 1 && (len == 1)) {
        // change LEDs of indicator
        delegateLedUsb(data[0]);
        expectReport = 0;
    }else if (expectReport == 2){   // options
        DBG1(0xEE, data, len);
        // start bootloader
        if(data[1] == OPTION_INDEX_BOOTLOADER && len == 8){
        	if(data[2] == 0xFF){
        		eeprom_write_byte((uint8_t *)EEPROM_BOOTLOADER_START, 0x00);
        	}
        	delegateGotoBootloader();
#ifdef ENABLE_BOOTMAPPER
        }else if(data[1] == OPTION_INDEX_BOOTMAPPER){
        	if(data[2] == OPTION_VALUE_BOOTMAPPER_START){
        		setToBootMapper(true);
        	}else{
        		setToBootMapper(false);
        	}
#endif
        }else if(data[1] == OPTION_INDEX_READY){
        	//stop timer
        	// stop timer1
        	stopFullLed();

        }else if(data[1] == OPTION_INDEX_ACTION){
        	// start timer1
        	startFullLed();

        }else{
        	setLed2((uint8_t *)data);
        }

//        expectReport = 0;
    }else if (expectReport == 4){
    	// rainbow color setting
    	DBG1(0x44, data, len);
        setLed2((uint8_t *)data);
    }else if (expectReport == 3){   // HID_REPORT_BOOT
        DBG1(0xDD, data, len);
        /*
         * 44: 02 10 00 00 00 00 00 00
        44: ff 00 ff 00 ff 00 00 00
        44: 00 00 00 00 00 00 00 00
        44: 00 00 00 00 00 00 00 00
        44: 00 00 00
         */
//        uchar   isLast;
//        static uchar            offset;
//        if(isStart == 1){
//        	isStart = 0;
//			offset = 0;
//			len -= 4;
//		}
//		offset += len;
//		isLast = offset & 0x80; /* != 0 if last block received */
//		DBG1(0xDE, (void *)&isLast, 1);
//		return isLast;

       /* uint8_t gRet;

        gRet = writeFlash(data, len, isStart);
        if(isStart == 1){
            isStart = 0;
        }

        DBG1(0xDF, (uchar *)&gRet, 1);
        return gRet;*/

        /*
         *
         *
         * 여기까지 제대로 실행이 되지만, '커뮤니케이션 에러'가 발생하면서 다음 page로 진행이 되지 않는다.
         * - 일단 page는 저장이 된 상태
         *
         * USB_CFG_SUPPRESS_INTR_CODE 와 USB_CFG_INTR_POLL_INTERVAL 값을 변경해야 통신이 원활히 된다.
         * (아마도 다른 인터럽트를 허용하지 않고 해당 통신만 전담하는 세팅인것 같다.)
         *
         * USB_CFG_INTR_POLL_INTERVAL를 높히면 매크로 출력시 속도가 느리고,
         * USB_CFG_SUPPRESS_INTR_CODE를 1로 설정하면 키보드로 작동을 안한다.
         *
         * 이 둘을 최적화 했다 치더라도, 통신 중(페이지 단위로 쓰기 위해서 8바이트씩 전송되는 데이터를 조합하고 이 페이지를 5개 받는다)에
         * 플래시를 쓰면 커뮤니케이션 에러가 발생하고,
         *
         * 이를 방지하기 위해서 모든 페이지를 메모리에 저장 후 쓰려고 하면, 작동은하지만 메모리가 부족한 현상이 있다.
         *
         *
         *
         *
         * */
    }

    return 0x01;
}


