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
#include <stdbool.h>
#include <string.h>
#include <util/delay.h>

//#include <avr/wdt.h>
#include "usbdrv/usbdrv.h"

#include "hardwareinfo.h"
#include "keymatrix.h"
#include "ledrender.h"
#include "keymap.h"
#include "keymapper.h"
#include "keyindex.h"
#include "macrobuffer.h"
#include "sleep.h"
#include "enterframe.h"
#include "keydownbuffer.h"
#include "keymain.h"
#include "fncontrol.h"
#include "vusb.h"
#include "ps2avru_util.h"
#include "dualaction.h"
#include "smartkey.h"

#define REPORT_ID_INDEX 0
#define KEYBOARD_MODIFIER_INDEX 0

#define INIT_INDEX_NOT_INIT     0
#define INIT_INDEX_SET_IDLE     1
#define INIT_INDEX_INITED       2
#define INIT_INDEX_COMPLETE     3

static uint8_t _initState = INIT_INDEX_NOT_INIT;     // set 1 when HID init
static uint8_t _ledInitState = INIT_INDEX_NOT_INIT;
static int initCount = 0;

// static uint8_t _prevPressedBuffer[MACRO_SIZE_MAX];
uint8_t macroBuffer[REPORT_SIZE_KEYBOARD];

void clearReportBuffer(void);
void wakeUpUsb(void);
void countSleepUsb(void);

void delegateLedUsb(uint8_t xState){
    setLEDState(xState); // Get the state of all LEDs
    setLEDIndicate();  
    if(_ledInitState == INIT_INDEX_NOT_INIT){
        _ledInitState = INIT_INDEX_INITED;
    }
}

void delegateInterfaceReadyUsb(void){
    interfaceReady = 1;
}

void delegateInitInterfaceUsb(void)
{    
    // 부팅시 cmos와 os에서 각각 불려지므로, os 시작시에 초기화를 해주려면 if문 없이 실행해야한다.
    // if(_initState == INIT_INDEX_NOT_INIT || _initState == INIT_INDEX_COMPLETE) {
        _initState = INIT_INDEX_SET_IDLE;
        _ledInitState = INIT_INDEX_NOT_INIT;       
    // }
}

/*void delegateStopTimerWhenUsbInterupt(void){
    stopTimer();
}
void delegateStartTimerWhenUsbInterupt(void){
    startTimer();
}*/

/*void prepareKeyMappingUsb(void)
{
    
}*/

/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  USB  ----------------------------- */
/* ------------------------------------------------------------------------- */
uint8_t reportIndex; // reportBuffer[KEYBOARD_MODIFIER_INDEX] contains modifiers
uint8_t _extraHasChanged = 0;
uint8_t makeReportBufferExtra(uint8_t keyidx, bool xIsDown){
    if(keyidx > KEY_Multimedia && keyidx < KEY_Multimedia_end){

        _extraHasChanged = 1;
        uint16_t gKeyidxMulti = pgm_read_word(&keycode_USB_multimedia[keyidx - (KEY_Multimedia + 1)]);
        reportBufferExtra[REPORT_ID_INDEX] = REPORT_ID_CONSUMER;
        if(xIsDown){
            reportBufferExtra[REPORT_ID_INDEX + 1] = (uint8_t)(gKeyidxMulti & 0xFF);
            reportBufferExtra[REPORT_ID_INDEX + 2] = (uint8_t)((gKeyidxMulti >> 8) & 0xFF);
        }else{
            reportBufferExtra[REPORT_ID_INDEX + 1] = 0;
            reportBufferExtra[REPORT_ID_INDEX + 2] = 0;
            
        }
        
        return 1;
    }
    return 0;
}
uint8_t makeReportBuffer(uint8_t keyidx, bool xIsDown){
    uint8_t retval = 1;

    // 듀얼액션 취소되었을 때는 down 키코드를 적용한다.;
    keyidx = getDualActionDownKeyIndexWhenIsCancel(keyidx);      

    // DEBUG_PRINT(("key down!!! keyidx : %d , reportIndex : %d \n", keyidx, reportIndex));

    if(keyidx >= KEY_MAX){
        return 0;       
    }else if(keyidx > KEY_Multimedia && keyidx < KEY_Multimedia_end){
        return 0;
    }else if (keyidx > KEY_Modifiers && keyidx < KEY_Modifiers_end) { /* Is this a modifier key? */
        reportBuffer[KEYBOARD_MODIFIER_INDEX] |= modmask[keyidx - (KEY_Modifiers + 1)];

        return retval;
    }

    if (keyidx != KEY_NONE) { // keycode should be added to report
        if (reportIndex >= REPORT_SIZE_KEYBOARD) { // too many keycodes
            // DEBUG_PRINT(("too many keycodes : reportIndex = %d \n", reportIndex));
            // if (!retval & 0x02) { // Only fill buffer once
                // memset(reportBuffer+2, KEY_ErrorRollOver, sizeof(reportBuffer)-2);
                /*6키 이상 입력시 새로운 키는 무시되지만 8키를 누른 후 
                6번째 이전의 키 하나를 떼도 7키가 눌린것으로 판단 이전 6개의 키가 유지되는 버그가 있어서 매트릭스 순으로 6개를 처리하도록 방치;*/
                // retval |= 0x02; // continue decoding to get modifiers
            // }
        } else {
            // DEBUG_PRINT(("reportBuffer[reportIndex] : reportIndex = %d keyidx = %d \n", reportIndex, keyidx));
            if(keyidx > KEY_extend && keyidx < KEY_extend_end){
                keyidx = pgm_read_byte(&keycode_USB_extend[keyidx - (KEY_extend + 1)]); 
            }
            reportBuffer[reportIndex] = keyidx; // set next available entry
            reportIndex++;
        }
    }

    return retval;
}

uint8_t makeReportBufferDecorator(uint8_t keyidx, bool xIsDown){
    
    if(xIsDown){ 
        // 듀얼액션 취소되었을 때는 down 키코드를 적용한다.;       
        pushDownBuffer(getDualActionDownKeyIndexWhenIsCancel(keyidx));
    }

    return makeReportBuffer(keyidx, xIsDown);

}

void clearReportBuffer(void){    
    memset(reportBuffer, 0, sizeof(reportBuffer)); // clear report buffer 
    memset(reportBufferExtra, 0, sizeof(reportBufferExtra));
}

uint8_t scanKeyUSB(void) {
    uint8_t retval = 0;

	// debounce cleared
    if (!setCurrentMatrix()) return retval;    

	uint8_t row, col, prev, cur, keyidx, gKeymapping;
    bool gFN;
	uint8_t gLayer = getLayer();

    DEBUG_PRINT(("gLayer  : %d \n", gLayer)); 

	// debounce counter expired, create report
	reportIndex = 2;
    clearReportBuffer();
    clearDownBuffer();

    _extraHasChanged = 0;

    gKeymapping = 0;
    uint8_t *gMatrix = getCurrentMatrix();
    uint8_t *gPrevMatrix = getPrevMatrix();
	for (col = 0; col < COLUMNS; ++col) { // process all rows for key-codes
		for (row = 0; row < ROWS; ++row) { // check every bit on this row   

			// usb 입력은 눌렸을 때만 확인하면 되지만, 각종 FN키 조작을 위해서 업/다운을 모두 확인한다.
			prev = gPrevMatrix[row] & BV(col);
			cur  = gMatrix[row] & BV(col);
            keyidx = getCurrentKeyindex(gLayer, row, col);	   		
			gFN = true;
            
            // !(prev&&cur) : 1 && 1 이 아니고, 
            // !(!prev&&!cur) : 0 && 0 이 아니고, 
            // 이전 상태에서(press/up) 변화가 있을 경우;
			//if( !(prev&&cur) && !(!prev&&!cur) && keyidx != KEY_NONE ) {                
            if( prev != cur ) {

#ifdef ENABLE_BOOTMAPPER           
                if(isBootMapper()){
                    if(cur) trace(row, col);
                    wakeUpUsb();
                    break;
                }
#endif      
                if(keyidx != KEY_NONE){
    				if(cur) {
    					// key down
    					gFN = applyFN(keyidx, col, row, true);
                        wakeUpUsb();
                        applyDualActionDownWhenIsCancel(makeReportBufferDecorator, true);

    				}else{
                        // key up
    					gFN = applyFN(keyidx, col, row, false);
    				}
                }
			}

            // 키매핑 진행중;
            if(prev != cur && isKeyMapping()){               
                if(cur){
                    putKeyindex(keyidx, col, row, 1);
                }else{
                    putKeyindex(keyidx, col, row, 0);
                }
                gKeymapping = 1;
                
                continue;
            }
            
            if(cur && keyidx != KEY_NONE && applyMacro(keyidx)) {
                // 매크로 실행됨;
                return 0;
            }
            
            // fn키를 키매핑에 적용하려면 위치 주의;
            if(gFN == false) continue;

			// usb는 눌렸을 때만 버퍼에 저장한다.
			if(cur){
               //DEBUG_PRINT(("key down!!! keyidx : %d , reportIndex : %d \n", keyidx, reportIndex));
               retval |= makeReportBufferDecorator(keyidx, true);
			}

            if( prev != cur ) {
                if(cur){
                    makeReportBufferExtra(keyidx, true);
                }else{
                    makeReportBufferExtra(keyidx, false);
                }

            }
			
		}
	}

	retval |= 0x01; // must have been a change at some point, since debounce is done
	
    setPrevMatrix();

    if(gKeymapping == 1) return 0;
	
    return retval;
}

static uint8_t _needRelease = 0;
void clearMacroBuffer(void){    
    memset(macroBuffer, 0, sizeof(macroBuffer)); 
}
uint8_t scanMacroUsb(void)
{
    clearReportBuffer(); 

    // 매크로 실행시 modifier 키를 함께 사용할 수 있도록 스캔;
    if(!isKeyMapping()
#ifdef ENABLE_BOOTMAPPER           
        && !isBootMapper()
#endif    
        ){
        uint8_t row, col, cur, keyidx;
        uint8_t *gMatrix = getCurrentMatrix();
        uint8_t keymap = getLayer();
        for (col = 0; col < COLUMNS; ++col) { // process all rows for key-codes
            for (row = 0; row < ROWS; ++row) { // check every bit on this row   
               
                cur  = gMatrix[row] & BV(col);           
                // usb는 눌렸을 때만 버퍼에 저장한다.
                if(cur){
                    keyidx = getCurrentKeyindex(keymap, row, col);
                    if (keyidx > KEY_Modifiers && keyidx < KEY_Modifiers_end) {
                        reportBuffer[KEYBOARD_MODIFIER_INDEX] |= modmask[keyidx - (KEY_Modifiers + 1)];
                    }
                }
                
            }
        }
    }

    if(!isEmptyM()){
        int gIdx;  
        Key gKey;    
        uint8_t i;
        uint8_t gLen = strlen((char *)macroBuffer);
        uint8_t gKeyidx;  
      
        gKey = popMWithKey();  
        
        if(gKey.mode == 1){    // down
            append(macroBuffer, gKey.keyindex);

            makeReportBufferExtra(gKey.keyindex, true);
        }else{  // up
            gIdx = findIndex(macroBuffer, gLen, gKey.keyindex);
            if(gIdx > -1){
                delete(macroBuffer, gIdx);

                makeReportBufferExtra(gKey.keyindex, false);
            }
        }

        reportIndex = 2;
        gLen = strlen((char *)macroBuffer);
        for (i = 0; i < gLen; ++i)
        {
            gKeyidx = macroBuffer[i];
            makeReportBuffer(gKeyidx, 1); 
        }
        _needRelease = 1;

      
    }else if(_needRelease){ 
        clearMacroBuffer();
        _needRelease = 0;
    }

    return 1;

}
uint8_t hasMacroUsb(void)
{
    return (_needRelease || !isEmptyM());
}

void wakeUpUsb(void){
    #if !USB_COUNT_SOF  
        wakeUp();
    #endif
}
void countSleepUsb(void){
    #if !USB_COUNT_SOF  
        countSleep();
    #endif    
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
    clearMacroBuffer();

    uchar   i = 0;
    usbDeviceDisconnect();  /* do this while interrupts are disabled */
    do{             /* fake USB disconnect for > 250 ms */
        // wdt_reset();
        _delay_ms(1);
    }while(--i);
    usbDeviceConnect();
    
	DEBUG_PRINT(("starting USB keyboard!!! \n"));

    sei();

#if USB_COUNT_SOF    
    bool _isSuspended = false;
    int suspendCount = 0;
#endif
    
    while (1) {

		// 카운트 이내에 신호가 잡히지 않으면 이동;
		// 특별한 경우에만 발생하는 현상이다.
		if(INTERFACE == INTERFACE_USB && interfaceReady == 0 && interfaceCount++ > 2000){
			// move to ps/2
			INTERFACE = 0;
			DEBUG_PRINT(("               move to ps/2 \n"));
			break;
		}

#if USB_COUNT_SOF
        if (usbSofCount != 0) {
            // if(_isSuspended == true) DEBUG_PRINT(("_isSuspended == true usbSofCount : %d \n", usbSofCount));
            _isSuspended = false;
            usbSofCount = 0;
            suspendCount = 0;

            wakeUp();

        } else {
            // Suspend when no SOF in 3ms-10ms(7.1.7.4 Suspending of USB1.1)
            if (_isSuspended == false && suspendCount++ > 2000) {
                // DEBUG_PRINT(("_isSuspended == false usbSofCount : %d \n", usbSofCount));
                suspendCount = 0;
                _isSuspended = true;

                sleep();
            }
        }
#endif

        // main event loop
        usbPoll();

#if USB_COUNT_SOF 
        if(_isSuspended == true) {
            scanKeyUSB();   // for dummy
            continue;
        }
#endif

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

        // ps2avrU loop, must be after scan matrix;
        enterFrame();

        // if an update is needed, send the report
        if (usbInterruptIsReady()) {  
            if(_initState == INIT_INDEX_SET_IDLE){
                _initState = INIT_INDEX_INITED;
                clearReportBuffer();
                usbSetInterrupt(reportBuffer, REPORT_SIZE_KEYBOARD);   // 재부팅시 첫키 입력 오류를 방지하기 위해서 HID init 후 all release 전송; 

                wakeUpUsb(); 
                
                // 플러깅 후 출력되는 메세지는 넘락등 LED가 반응한 후에 보여진다. 
                // usbInterruptIsReady() 일지라도 LED 반응 전에는 출력이 되지 않는다.
                // LED 반응 후에 처리하려고 하면 MAC OS에서 실행되지 않는다.
                // (MAC OS에서는 플러깅 시 LED가 반응하지 않는다. 대신 바로 출력이 된다.)
                // for os x
                if(idleRate > 0) {
                    startKeyMappingOnBoot();

                    setCurrentOS(true);
                }

            }else if(_ledInitState == INIT_INDEX_INITED){
                _ledInitState = INIT_INDEX_COMPLETE;  
                // for windows
                if(idleRate == 0) startKeyMappingOnBoot();
                
            }else if(hasMacroUsb()){
                scanMacroUsb();
                usbSetInterrupt(reportBuffer, REPORT_SIZE_KEYBOARD);
            }else if(updateNeeded && !_extraHasChanged){                 
                usbSetInterrupt(reportBuffer, REPORT_SIZE_KEYBOARD);                 
                updateNeeded = 0;
            }
        }

        if(_initState == INIT_INDEX_INITED){
            if(initCount++ == 200){       // delay for OS X USB multi device   
                // all platform init led
                initFullLEDState();
                _initState = INIT_INDEX_COMPLETE;
            }
        }

        if(usbInterruptIsReady3()){
            if(updateNeeded && _extraHasChanged){ 
                usbSetInterrupt3(reportBufferExtra, REPORT_SIZE_EXTRA);    
                updateNeeded = 0;
            }
        }
        
        // 입력이 한동안 없으면 슬립모드로;
        countSleepUsb();        
           
		// interrupt 위에서 실행되면 status LED가 제대로 반응하지 않는다.
		renderLED();
        
        if(hasMacroUsb())
        {
            setMacroProcessEnd(false);
        }else{
            setMacroProcessEnd(true);
        }

    }

	// data line reset;
	USB_INTR_ENABLE &= ~(1 << USB_INTR_ENABLE_BIT);

	return; // 0;
}
#endif
