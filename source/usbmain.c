/**
 * \file firmware/main.c
 * \brief Main functions for USB-keyboard
 * \author Ronald Schaten <ronald@schatenseite.de>
 * \version $Id: main.c,v 1.5 2008-07-12 21:05:24 rschaten Exp $
 *
 * License: GNU GPL v2 (see License.txt)
 */

/**
 * \mainpage Dulcimer
 *
 * \section sec_intro Introduction
 *
 * A computer keyboard can be a very personal utensil. Especially if it is an
 * extraordinary well built one, like for example the IBM Model M. The Model M
 * design dates back to 1984, but it still has many fans even nowadays. It came
 * with the usual keyboard connectors. First the old 5-pin one, later a PS/2
 * plug. Unfortunately is that, at least to my knowledge, they never released a
 * version with USB.
 *
 * A friend of mine knew that I already had built other USB-devices, and one of
 * them even acted as a keyboard (it isn't really a keyboard, but that's a
 * different story... ;-) ). He is a big fan of the Model M, so he asked if I
 * could put new life in one of his old keyboards, which had a broken circuit
 * inside. And this is the result...
 *
 * \subsection sec_technique Hard- and Software
 *
 * The main part of a computer keyboard circuit is the key matrix. You can
 * imagine it as a number of keys, placed on a raster of horizontal (rows) and
 * vertical (columns) wires. In the case of a Model M keyboard, we have a
 * matrix of 8x16 lines. Eight columns in 16 rows, or the other way around,
 * depending on how you look at it. Each key is connected to one column and one
 * row. If you press the key, it will connect the column and the row on it's
 * crossing of the lines.
 *
 * Connected to this matrix is a keyboard controller. That's a chip with a
 * number of I/O-lines to detect the state of the matrix, and on the other side
 * an interface that enables it to talk to the computer. Oh, and not to forget:
 * it also has three output lines to drive the LEDs for Num-, Caps- and
 * Scroll-lock.
 *
 * What I did in this project is, that I dumped the keyboard controller chip
 * and its circuit, and replaced it by an ATmega32 and my own circuit. The
 * ATmega scans the matrix for keyboard activity, controls the LEDs and talks
 * to the computer.
 *
 * For further convenience, I added a boot-loader. With that, it is possible to
 * update the keyboard's firmware without disassembling it, and without the
 * need for a dedicated programmer.
 *
 * \subsection sec_hardware Other hardware?
 *
 * As mentioned, the controller in this project is just connected to an
 * ordinary keyboard matrix. You can find this kind of matrix in all kinds of
 * keyboards, from key-telephones over good old hardware like the Commodore
 * C=64 or the Schneider CPC, keyboards with non-PC-connectors like those made
 * by Sun, to modern hardware that could need a few more features.
 *
 * Till now, I just made a PCB layout for the IBM Model M, but I intend to
 * modify at least a Sun keyboard. In order to do that, I expect having to
 * refactor the key-scanning, since the key-matrix is not 16x8. The positions
 * of the keys on the matrix will be different, I'll have to re-engineer that.
 * And of course, I'll have to make another PCB.
 *
 * \subsection sec_features Features
 *
 * At the moment, the keyboard should be able to do everything that the average
 * off-the-shelf-keyboard can do. But there are many features that are
 * possible, regarding the fact that the ATmega32 is absolutely bored till now.
 * You can think of 'magic keystrokes' that turn some hidden features on or
 * off, like for example:
 *  - send complete phrases on one keystroke
 *  - 'autofire' feature on keys that don't repeat normally, for example Alt+F4
 *  - change keyboard layout without reconfiguring the computer
 *  - turn bouncing keys on or off, to annoy other people using your computer
 *  - random caps lock function
 *  - use arrow keys as mouse, without having to include a special driver in
 *    the OS.
 *
 * With a little tweaking on the hardware side, there should be even more
 * possibilities:
 *  - turn the oldtimer-keyboard into a supermodern wireless bluetooth one
 *  - implement keylogger-funktionality, using for example an SD-card
 *  - include an USB-hub into the keyboard
 *
 * If you are just a little like me, it won't take you much brainstorming to
 * come up with own useful -- or even better: useless -- ideas. ;-)
 *
 * \section sec_install Building and installing
 *
 * Both, the bootloader and firmware are simply built with "make". You may need
 * to customize both makefiles to fit to your system. If you don't want to add
 * new features, you don't need to build the software yourself. You can use the
 * hex-files included in this package.
 *
 * \subsection sec_boot Bootloader
 *
 * I used the USBaspLoader from Objective Development, the same guys that wrote
 * the AVR-USB-driver: http://www.obdev.at/products/avrusb/usbasploader.html
 *
 * The reason why I chose this over some other available USB-bootloaders is,
 * that this one emulates a common ISP-programmer that is supported by avrdude.
 * In this way, the same program can be used to program the chip that is used
 * without a bootloader.
 *
 * To prepare the ATmega32, you have to connect it to your computer with the
 * ISP-programmer of your choice and modify the makefile according to that.
 * Then you enter the bootloader-directory and enter the following line:
 *
 * \code
 * make fuse && make flash && make lock
 * \endcode
 * 
 * With 'fuse' you prepare the fuse-bits of your AVR, 'flash' transfers the
 * bootloader to the device and 'lock' prevents you from overwriting the
 * bootloader. Don't fear the locking: you can always reset it with your
 * ordinary programmer. In fact, it is disabled in the moment you use your
 * ordinary programmer to reflash the device, even without any special
 * parameters. The locking only affects the bootloader behavior.
 *
 * Afterwards you can put the programmer back into the toolbox, you won't need
 * it from here on.
 *
 * When you plug in the device while holding the minus-key on the number-keypad
 * pressed, the keyboard indicates that it would like to get a new firmware by
 * showing a running light on the LEDs. That firmware will be flashed over the
 * normal USB-cable that the keyboard is connected with.
 *
 * \subsection sec_fw Firmware
 *
 * If you intend to recompile the firmware yourself, you will need avr-gcc and
 * avr-libc (a C-library for the AVR controller). Please read the instructions
 * at http://www.nongnu.org/avr-libc/user-manual/install_tools.html for how to
 * install the GNU toolchain (avr-gcc, assembler, linker etc.) and avr-libc.
 *
 * Once you have the GNU toolchain for AVR microcontrollers installed, you can
 * run "make" in the subdirectory "firmware".
 *
 * Afterwards -- or if you decided not to compile the firmware yourself -- you
 * can flash it to the device:
 *
 * \code
 * make program
 * \endcode
 *
 * Remember that you have to start the bootloader at first: unplug the
 * keyboard, hold the minus-key on the number-keypad pressed and replug it. If
 * the modified keyboard is the only one within reach: good luck! ;-)
 *
 * \section sec_usage Usage
 *
 * Connect the keyboard to the USB-port. All LED should flash up to indicate
 * that the device is initialized.
 *
 * Then you can use the keyboard as always. If additional features get
 * implemented, you will be able to use them in their respective ways.
 *
 * \section sec_drawbacks Drawbacks
 *
 * I don't know if and how keyboard manufacturers face the problem of ghost
 * keys, I didn't take special measurements for this. I hope that the engineers
 * at IBM distributed the keys on the matrix in a way that minimizes this
 * problem. Don't misunderstand: I haven't experienced that on this keyboard,
 * but I know that it's a common problem on key-matrixes.
 *
 * \section sec_files Files in the distribution
 *
 * - \e Readme.txt: Documentation, created from the htmldoc-directory.
 * - \e firmware: Source code of the controller firmware.
 * - \e firmware/usbdrv: USB driver -- See Readme.txt in this directory for
 *   info.
 * - \e bootloader: The USBaspLoader, properly configured for this project. I
 *   only modified the bootloaderconfig.h and the Makefile.
 * - \e USBaspLoader.2008-02-05.tar.gz: The unmodified bootloader sources, for
 *   reference.
 * - \e circuit: Circuit diagrams in PDF and KiCAD format. KiCAD is a free
 *   schematic- and layout-tool, you can learn more about it at its homepage:
 *   http://www.lis.inpg.fr/realise_au_lis/kicad/
 * - \e License.txt: Public license for all contents of this project, except
 *   for the USB driver. Look in firmware/usbdrv/License.txt for further info.
 * - \e Changelog.txt: Logfile documenting changes in soft-, firm- and
 *   hardware.
 * - \e refman.pdf: Full documentation of the software.
 *
 * \section sec_thanks Thanks!
 *
 * I'd like to thank <b>Objective Development</b> for the possibility to use
 * their driver for my project. In fact, this project wouldn't exist without
 * the driver.
 *
 * And of course I'd like to thank that friend of mine -- I doubt that he'd
 * like to read his name in this place, I'll put it in if he wants me to --
 * that gave me the idea for this project.
 *
 * \section sec_license About the license
 *
 * My work - all contents except for the USB driver - is licensed under the GNU
 * General Public License (GPL). A copy of the GPL is included in License.txt.
 * The driver itself is licensed under a special license by Objective
 * Development. See firmware/usbdrv/License.txt for further info.
 *
 * <b>(c) 2008 by Ronald Schaten - http://www.schatenseite.de</b>
 */


#ifndef KEYBD_EXTERN_USB
#define KEYBD_EXTERN_USB

#include "usbmain.h"

#include "global.h"
#include "timer.h"
#include "print.h"
#include "keymap.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

//#include <avr/wdt.h>
#include "usbdrv/usbdrv.h"

#include "hardwareinfo.h"
#include "keymatrix.h"
#include "ledrender.h"
#include "keymapper.h"
#include "macrobuffer.h"
#include "sleep.h"

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

static uint8_t reportBuffer[8]; ///< buffer for HID reports
static uint8_t idleRate;        ///< in 4ms units
static uint8_t protocolVer = 1; ///< 0 = boot protocol, 1 = report protocol
static uint8_t expectReport = 0;       ///< flag to indicate if we expect an USB-report
static uint8_t _isInit = 0;     // set 1 when HID init

static uint8_t _prevPressedBuffer[MACRO_SIZE_MAX];

#define REPORT_ID_KEYBOARD      1
#define REPORT_ID_MULTIMEDIA    2

void initInterfaceUsb(void);

/** USB report descriptor (length is defined in usbconfig.h). The report
 * descriptor has been created with usb.org's "HID Descriptor Tool" which can
 * be downloaded from http://www.usb.org/developers/hidpage/ (it's an .exe, but
 * it even runs under Wine).
 */
PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {

    0x05, 0x01,   // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,   // USAGE (Keyboard)
    0xa1, 0x01,   // COLLECTION (Application)
    0x85, 0x01,               //Report ID (1)
    0x05, 0x07,   //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,   //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,   //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,   //   LOGICAL_MINIMUM (0)
    0x25, 0x01,   //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,   //   REPORT_SIZE (1)
    0x95, 0x08,   //   REPORT_COUNT (8)
    0x81, 0x02,   //   INPUT (Data,Var,Abs)
    /*0x95, 0x01,   //   REPORT_COUNT (1)
    0x75, 0x08,   //   REPORT_SIZE (8)
    0x81, 0x03,   //   INPUT (Cnst,Var,Abs)*/ 
    // endpoint가 1개 일때는 [modi, reserved, keycode, ...] 순이었지만 
    //endpoint가 2개일때는 [Report ID, modi, keycode, ...]순이므로 reserved 는 제거;
    0x95, 0x05,   //   REPORT_COUNT (5)
    0x75, 0x01,   //   REPORT_SIZE (1)
    0x05, 0x08,   //   USAGE_PAGE (LEDs)
    0x19, 0x01,   //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,   //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,   //   OUTPUT (Data,Var,Abs)
    0x95, 0x01,   //   REPORT_COUNT (1)
    0x75, 0x03,   //   REPORT_SIZE (3)
    0x91, 0x03,   //   OUTPUT (Cnst,Var,Abs)
    0x95, 0x06,   //   REPORT_COUNT (6)
    0x75, 0x08,   //   REPORT_SIZE (8)
    0x15, 0x00,   //   LOGICAL_MINIMUM (0)
    0x26, 0x91, 0x00,  //   LOGICAL_MAXIMUM (145)
    0x05, 0x07,   //   USAGE_PAGE (Keyboard)
    0x19, 0x00,   //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0xA4,   //   USAGE_MAXIMUM (Keyboard ExSel)
    0x81, 0x00,   //   INPUT (Data,Ary,Abs)
    0xc0,                           // END_COLLECTION

    0x05, 0x0c,                    // USAGE_PAGE (Consumer Devices)
    0x09, 0x01,                    // USAGE (Consumer Control)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x02,                    //   REPORT_ID (2)
    0x19, 0x00,                    //   USAGE_MINIMUM (Unassigned)
    0x2a, 0x3c, 0x02,              //   USAGE_MAXIMUM (AC Format)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0x3c, 0x02,              //   LOGICAL_MAXIMUM (572)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x10,                    //   REPORT_SIZE (16)
    0x81, 0x00,                    //   INPUT (Data,Var,Abs)
    0xc0                           // END_COLLECTION
};


/**
 * This function is called whenever we receive a setup request via USB.
 * \param data[8] eight bytes of data we received
 * \return number of bytes to use, or 0xff if usbFunctionWrite() should be
 * called
 */
uint8_t usbFunctionSetup(uint8_t data[8]) {

    interfaceReady = 1;

    usbRequest_t *rq = (void *)data;
    usbMsgPtr = *reportBuffer;

    // DEBUG_PRINT(("USBRQ_TYPE : %d \n", (rq->bmRequestType & USBRQ_TYPE_MASK)));

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {

        // DEBUG_PRINT(("rq->bRequest : %d \n", rq->bRequest));

        // class request type
        if (rq->bRequest == USBRQ_HID_GET_REPORT) {
            // wValue: ReportType (highbyte), ReportID (lowbyte)
            // DEBUG_PRINT(("rq->wValue[0] : %d rq->wValue[1] : %d \n", rq->wValue.bytes[0], rq->wValue.bytes[1]));
            // we only have one report type, so don't look at wValue
            return sizeof(reportBuffer);
        } else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
            // MAC OS X is not processing here

            //DEBUG_PRINT(("rq->wLength.word : %d rq->wValue : %d  rq->wIndex : %d\n", rq->wLength.word, rq->wValue, rq->wIndex));
            if (rq->wLength.word == 2) {
                // We expect one byte reports
                expectReport = 1;
                return 0xff; // Call usbFunctionWrite with data
            }
        } else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
            usbMsgPtr = idleRate;
            return 1;
        } else if (rq->bRequest == USBRQ_HID_SET_IDLE) {

			// init full led
            initInterfaceUsb();

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
    //DEBUG_PRINT(("usbFunctionWrite len : %d \n", len));
    // DEBUG_PRINT(("data[0] : %d data[1] : %d data[2] : %d \n", data[0], data[1], data[2]));
    // if (expectReport && (len == 1)) {
    if (expectReport && (len == 2) && data[0] == 1) {
		// setLEDState(data[0]); // Get the state of all LEDs
        setLEDState(data[1]); // Get the state of all LEDs
		setLEDIndicate();	
    }
    expectReport = 0;
    return 0x01;
}

void initInterfaceUsb(void)
{    
    _isInit = 1;
    initFullLEDState();
    wakeUp();            
}

void prepareKeyMappingUsb(void)
{
    memset(_prevPressedBuffer, 0, MACRO_SIZE_MAX);
}

//     /*
//         "스트링을 입력받아 키코드로 변환"은 외부에서 진행하고 
//         메세지 버퍼에 저장하고, 메세지가 있음을 표시하고, 순서대로 전송한다.
//         모두 전송하면 더이상 메세지가 없다고 표시; 

//         매크로 구상 : 모든 키는 누르고 떼는 동작만 있다. 때문에 처음 키코드는 누른 것으로 인식 2번째 키코드는 뗀 것으로 인식하면되겠다. 
//     */


/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  USB  ----------------------------- */
/* ------------------------------------------------------------------------- */
uint8_t reportIndex; // reportBuffer[0] contains modifiers
uint8_t _isMultimediaPressed = 0;
uint8_t makeReportBuffer(uint8_t keyidx, uint8_t xIsDown){
    uint8_t retval = 0;

   
    // 듀얼액션 취소되었을 때는 다운 키코드를 적용한다.;
    keyidx = getDualActionDownKeyIdex(keyidx);      

    // DEBUG_PRINT(("key down!!! keyidx : %d , reportIndex : %d \n", keyidx, reportIndex));

    if(keyidx >= KEY_MAX){
        return retval;
    }else if(keyidx > KEY_Multimedia && keyidx < KEY_Multimedia_end){
        _isMultimediaPressed = 1;

        uint16_t gKeyidxMulti = pgm_read_word(&keycode_USB_multimedia[keyidx - (KEY_Multimedia + 1)]);
        reportBuffer[0] = REPORT_ID_MULTIMEDIA;  // ReportID = 2
        reportBuffer[1] = (uint8_t)(gKeyidxMulti & 0xFF);
        reportBuffer[2] = (uint8_t)((gKeyidxMulti >> 8) & 0xFF);

        return retval;
    }else if (keyidx > KEY_Modifiers && keyidx < KEY_Modifiers_end) { /* Is this a modifier key? */
        reportBuffer[1] |= modmask[keyidx - (KEY_Modifiers + 1)];

        //applyKeyMapping(reportBuffer[1]);
        return retval;
    }

    if (keyidx != KEY_NONE) { // keycode should be added to report
        if (reportIndex >= sizeof(reportBuffer)) { // too many keycodes
            //DEBUG_PRINT(("too many keycodes : reportIndex = %d \n", reportIndex));
            if (!retval & 0x02) { // Only fill buffer once
                memset(reportBuffer+2, KEY_ErrorRollOver, sizeof(reportBuffer)-2);
                retval |= 0x02; // continue decoding to get modifiers
            }
        } else {
            // DEBUG_PRINT(("reportBuffer[reportIndex] : reportIndex = %d keyidx = %d \n", reportIndex, keyidx));
            if(keyidx > KEY_extend && keyidx < KEY_extend_end){
                keyidx = pgm_read_word(&keycode_USB_extend[keyidx - (KEY_extend + 1)]); 
            }
            reportBuffer[reportIndex] = keyidx; // set next available entry
            reportIndex++;
        }
    }

    return retval;
}

void clearReportBuffer(void){    
    memset(reportBuffer, 0, sizeof(reportBuffer)); // clear report buffer
    reportBuffer[0] = REPORT_ID_KEYBOARD; // ReportID = 1
}

uint8_t scanKeyUSB(void) {
    uint8_t retval = 0;

	// debounce cleared
    if (!setCurrentMatrix()) return retval;    

	uint8_t row, col, prev, cur, keyidx, gKeymapping, gFN;
	uint8_t keymap = getLayer();
    static uint8_t prevModifier = 0;

	// debounce counter expired, create report
	reportIndex = 2; // reportBuffer[0] contains modifiers
    clearReportBuffer();
    gKeymapping = 0;
    uint8_t *gMatrix = getCurrentMatrix();
	for (col = 0; col < COLUMNS; ++col) { // process all rows for key-codes
		for (row = 0; row < ROWS; ++row) { // check every bit on this row   

			// usb 입력은 눌렸을 때만 확인하면 되지만, 각종 FN키 조작을 위해서 업/다운을 모두 확인한다.
			prev = prevMatrix[row] & BV(col);
			cur  = gMatrix[row] & BV(col);
            keyidx = getCurrentKeycode(keymap, row, col);						
			gFN = 1;
            
            if(cur && keyidx != KEY_NONE && applyMacro(keyidx)) {
                // 매크로 실행됨;
                return 0;
            }
            
            // !(prev&&cur) : 1 && 1 이 아니고, 
            // !(!prev&&!cur) : 0 && 0 이 아니고, 
            // 이전 상태에서(press/up) 변화가 있을 경우;
			//if( !(prev&&cur) && !(!prev&&!cur) && keyidx != KEY_NONE ) {                
            if( prev != cur ) {

#ifdef ENABLE_BOOTMAPPER           
                if(isBootMapper()){
                    if(cur) trace(row, col);
                    wakeUp();
                    break;
                }
#endif      
                if(keyidx != KEY_NONE){
    				if(cur) {
    					// key down
    					gFN = applyFN(keyidx, 1);
                        wakeUp();

                        /*if(dualActionKeyIndex > 0 && isCanceledDualAction()){
                            // 듀얼액션 활성화 후 다른 키가 눌려 취소되었을 때 우선 듀얼액션키의 down 값을 버퍼에 저장한다.
                            makeReportBuffer(getDualActionDownKeyIdex(dualActionKeyIndex), 1);
                            dualActionKeyIndex = 0;
                        }*/
                        applyDualActionDown(makeReportBuffer, 1);

    				}else{
                        // key up
    					gFN = applyFN(keyidx, 0);
    				}
                }
			}

            // 키매핑 진행중;
            if(prev != cur && isKeyMapping()){
                if(cur){
                    putKeyCode(keyidx, col, row, 1);
                }else{
                    putKeyCode(keyidx, col, row, 0);
                }
                gKeymapping = 1;
                
                continue;
            }
            
            // fn키를 키매핑에 적용하려면 위치 주의;
            if(gFN == 0) continue;

			// usb는 눌렸을 때만 버퍼에 저장한다.
			if(cur){
               //DEBUG_PRINT(("key down!!! keyidx : %d , reportIndex : %d \n", keyidx, reportIndex));
               makeReportBuffer(keyidx, 1);
			}
			
		}
	}

    if(reportBuffer[0] == REPORT_ID_KEYBOARD && reportBuffer[1] != prevModifier){
        applyKeyMapping(reportBuffer[1]);
        prevModifier = reportBuffer[1];
    }

// 멀티미디어 키 up
    if(_isMultimediaPressed && reportBuffer[0] != REPORT_ID_MULTIMEDIA){
        reportBuffer[0] = REPORT_ID_MULTIMEDIA;  // ReportID = 2 
        reportBuffer[1] = 0;
        reportBuffer[2] = 0; 
        _isMultimediaPressed = 0;
    }

    // 모든키가 release 되었을 때 작동 시켜 준다.  
   /* if((reportBuffer[0] == REPORT_ID_KEYBOARD && reportBuffer[1] == 0 && reportBuffer[2] == 0)
    || reportBuffer[0] == REPORT_ID_MULTIMEDIA && reportBuffer[1] == 0){
        applyDualAction();
    }*/
	retval |= 0x01; // must have been a change at some point, since debounce is done
	
	for(row=0;row<ROWS;++row)
		prevMatrix[row] = gMatrix[row];


    if(gKeymapping == 1) return 0;
	
    return retval;
}

static uint8_t _needRelease = 0;
uint8_t scanMacroUsb(void)
{
    if(!isEmptyM()){
        int gIdx;
        
        clearReportBuffer(); 
      
        popMWithKey();  // _pressedBuffer에 눌린 키들만 저장된다.        
        
        uint8_t i;
        uint8_t gLen = strlen((char *)_pressedBuffer);
        uint8_t gKeyidx;
        reportIndex = 2;
        for (i = 0; i < gLen; ++i)
        {
            gKeyidx = _pressedBuffer[i];
            // modi 가 아닌 놈들 중에 이미 press 리포트를 한 녀석은 다시 리포트에 포함 시키지 않는다.
            // 항상 이전 pressedBuffer를 가지고 있으면 된다.
            if (gKeyidx > KEY_Modifiers && gKeyidx < KEY_Modifiers_end){
                makeReportBuffer(gKeyidx, 1);                
            }else{
                gIdx = findIndex(_prevPressedBuffer, strlen((char *)_prevPressedBuffer), gKeyidx);
                if(gIdx == -1){
                   makeReportBuffer(gKeyidx, 1);
                }
            }
        }
        _needRelease = 1;

        memcpy (_prevPressedBuffer,_pressedBuffer,gLen+1);
      
    }else if(_needRelease){ 
        clearReportBuffer();
        _needRelease = 0;

    }

    return 1;

}
uint8_t hasMacroUsb(void)
{
    return (_needRelease || !isEmptyM());
}

/**
 * Main function, containing the main loop that manages timer- and
 * USB-functionality.
 * /return the obligatory integer that nobody cares about...
 */
void usb_main(void) {

    usbInit();

	uint8_t updateNeeded = 0;
    uint8_t idleCounter = 0;

    uchar   i = 0;
    usbDeviceDisconnect();  /* do this while interrupts are disabled */
    do{             /* fake USB disconnect for > 250 ms */
        // wdt_reset();
        _delay_ms(1);
    }while(--i);
    usbDeviceConnect();
    
	DEBUG_PRINT(("starting USB keyboard!!! \n"));

    sei();
    
	//scanKeyUSB();
    while (1) {

		// 카운트 이내에 신호가 잡히지 않으면 이동;
		// 특별한 경우에만 발생하는 현상이다.
		if(INTERFACE == INTERFACE_USB && interfaceReady == 0 && interfaceCount++ > 2000){
			// move to ps/2
			INTERFACE = 0;
			DEBUG_PRINT(("               move to ps/2 \n"));
			break;
		}

        // main event loop
        usbPoll();

        if(updateNeeded == 0){
		
            updateNeeded = scanKeyUSB(); // changes?
    		
            // check timer if we need periodic reports
            if (TIFR & (1 << TOV0)) {
                TIFR = (1 << TOV0); // reset flag
                if (idleRate != 0) { // do we need periodic reports?
                    if(idleCounter > 4){ // yes, but not yet
                        idleCounter -= 5; // 22ms in units of 4ms
                    } else { // yes, it is time now
                        updateNeeded = 1;
                        idleCounter = idleRate;
                    }
                }
            }
    	}else{
            scanKeyUSB();   // for dummy
        }

        // if an update is needed, send the report
        if (usbInterruptIsReady()) {   
            if(_isInit){
                _isInit = 0;
                clearReportBuffer();
                usbSetInterrupt(reportBuffer, 8);   // 재부팅시 첫키 입력 오류를 방지하기 위해서 HID init 후 all release 전송;
                
                // 플러깅 후 출력되는 메세지는 넘락등 LED가 반응한 후에 보여진다. 
                // usbInterruptIsReady() 일지라도 LED 반응 전에는 출력이 되지 않는다.
                // LED 반응 후에 처리하려고 하면 MAC OS에서 실행되지 않는다.(MAC OS에서는 플러깅 시 LED가 반응하지 않는다. 대신 바로 출력이 된다.)
                startKeyMappingOnBoot();
            }else if(hasMacroUsb()){
                scanMacroUsb();
                // DEBUG_PRINT(("hasMacroUsb\n"));
                usbSetInterrupt(reportBuffer, 8);
            }else if(updateNeeded){
                if(reportBuffer[0] == REPORT_ID_MULTIMEDIA){   // report ID : 2
                    // DEBUG_PRINT((" multi  reportBuffer : [0] = %d [1] = %d [2] = %d \n", reportBuffer[0], reportBuffer[1], reportBuffer[2]));
                    usbSetInterrupt(reportBuffer, 3);    
                }else if(reportBuffer[0] == REPORT_ID_KEYBOARD){ // report ID : 1
                    // DEBUG_PRINT((" updateNeeded : [0] = %d [1] = %d [2] = %d [3] = %d \n", reportBuffer[0], reportBuffer[1], reportBuffer[2], reportBuffer[3]));
                    usbSetInterrupt(reportBuffer, 8); 
                }
                updateNeeded = 0;
            }
        }
        
        // 입력이 한동안 없으면 슬립모드로;
        countSleep();        
           
		// interrupt 위에서 실행되면 status LED가 제대로 반응하지 않는다.
		renderLED(isBeyondFN);
        
        if(hasMacroUsb())
        {
            setMacroProcessEnd(0);
        }else{
            setMacroProcessEnd(1);
        }

    }

	// data line reset;
	USB_INTR_ENABLE &= ~(1 << USB_INTR_ENABLE_BIT);

	return; // 0;
}
#endif
