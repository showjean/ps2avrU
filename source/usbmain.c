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

#include "usbmain.h"

#include "global.h"
#include "timerinclude.h"
// #include "print.h"
#include "keymap.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <util/delay.h>

#include <avr/wdt.h>
#include "usbdrv/usbdrv.h"

#include "hardwareinfo.h"
#include "keymatrix.h"
#include "ledrender.h"
#include "keymap.h"
#include "quickmacro.h"
#include "keyindex.h"
#include "macrobuffer.h"
#include "sleep.h"
#include "enterframe.h"
#include "keydownbuffer.h"
#include "main.h"
#include "fncontrol.h"
#include "vusb.h"
#include "ps2avru_util.h"
#include "dualaction.h"
#include "keyscan.h"
#include "bootmapper.h"
#include "custommacro.h"
#include "oddebug.h"
//#include "boot.h"

#define REPORT_ID_INDEX 0
#define KEYBOARD_MODIFIER_INDEX 0

#define INIT_INDEX_NOT_INIT     0
#define INIT_INDEX_SET_IDLE     1
#define INIT_INDEX_INITED       2
#define INIT_INDEX_COMPLETE     3

#define INIT_LED_INDEX_NOT_INIT     0
#define INIT_LED_INDEX_SET_IDLE     1
#define INIT_LED_INDEX_INITED       2
#define INIT_LED_INDEX_COMPLETE     3

static uint8_t _initState = INIT_INDEX_NOT_INIT;     // set 1 when HID init
static uint8_t _ledInitState = INIT_LED_INDEX_NOT_INIT;
static int initCount = 0;
static bool _usbReset = false;

static void clearReportBuffer(void);
#if !USB_COUNT_SOF
static void wakeUpUsb(void);
static void countSleepUsb(void);
#endif
static void wake_up_signal(void);

void delegateLedUsb(uint8_t xState){
    setLEDState(xState); // Get the state of all LEDs
    setLEDIndicate();
    if(_ledInitState == INIT_LED_INDEX_NOT_INIT){
        _ledInitState = INIT_LED_INDEX_INITED;
    }
}

void delegateInterfaceReadyUsb(void){
    interfaceReady = true;
}

void delegateInitInterfaceUsb(void)
{    
    // 부팅시 cmos와 os에서 각각 불려지므로, os 시작시에 초기화를 해주려면 if문 없이 실행해야한다.
    // if(_initState == INIT_INDEX_NOT_INIT || _initState == INIT_INDEX_COMPLETE) {
        _initState = INIT_INDEX_SET_IDLE;
        _ledInitState = INIT_LED_INDEX_NOT_INIT;
        _usbReset = true;   
    // }
//   DBG1(0xA0, (uchar *)&idleRate, 1);
}


/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  USB  ----------------------------- */
/* ------------------------------------------------------------------------- */
// static uint8_t reportIndex; // reportBuffer[KEYBOARD_MODIFIER_INDEX] contains modifiers
static bool _extraHasChanged = false;
static uint16_t extraData = 0;
static bool _systemHasChanged = false;
static uint16_t systemData = 0;

static void makeReportBufferExtra(uint8_t keyidx, bool xIsDown){
    if(keyidx > KEY_Multimedia && keyidx < KEY_Multimedia_end){

        _extraHasChanged = true;

        if(xIsDown){
            extraData = pgm_read_word(&keycode_USB_multimedia[keyidx - (KEY_Multimedia + 1)]);
        }else{
            extraData = 0;
        }

    }
}
static void makeReportBufferSystem(uint8_t keyidx, bool xIsDown){
    if(keyidx >= KEY_POWER && keyidx <= KEY_WAKE){

        _systemHasChanged = true;

        if(xIsDown){
            systemData = pgm_read_word(&keycode_USB_multimedia[keyidx - (KEY_Multimedia + 1)]);
        }else{
            systemData = 0;
        }

    }
}
static uint8_t _modifiers = 0;
static uint8_t updateNeeded = 0;
static uint8_t reportBuffer[REPORT_KEYS];
static uint8_t _countOfBuffer = 0;


static void makeReportBuffer(uint8_t xKeyidx, bool xIsDown){

    if(xKeyidx == KEY_NONE || xKeyidx >= KEY_MAX || (xKeyidx > KEY_Multimedia && xKeyidx < KEY_Multimedia_end)){
        return;
    }

    updateNeeded = 1;

    uint8_t gLen;
    int gIdx;
    if(xIsDown){ 
        if (xKeyidx > KEY_Modifiers && xKeyidx < KEY_Modifiers_end) { /* Is this a modifier key? */
            _modifiers |= getModifierBit(xKeyidx);
        }else{ // keycode should be added to report
            gLen = _countOfBuffer; //strlen((char *)reportBuffer);
            if (gLen >= REPORT_KEYS) { // too many keycodes
                // if (!retval & 0x02) { // Only fill buffer once
                    // memset(reportBuffer+2, KEY_ErrorRollOver, sizeof(reportBuffer)-2);
                    /*6키 이상 입력시 새로운 키는 무시되지만 8키를 누른 후
                    6번째 이전의 키 하나를 떼도 7키가 눌린것으로 판단 이전 6개의 키가 유지되는 버그가 있어서 매트릭스 순으로 6개를 처리하도록 방치;*/
                    // retval |= 0x02; // continue decoding to get modifiers
                // }
            } else {
                if(xKeyidx > KEY_extend && xKeyidx < KEY_extend_end){
                    xKeyidx = pgm_read_byte(&keycode_USB_extend[xKeyidx - (KEY_extend + 1)]); 
                }
                gIdx = findIndex(reportBuffer, xKeyidx);
                if(gIdx == -1){
                    append(reportBuffer, xKeyidx);
                    _countOfBuffer++;
                }
            }
        }
    }else{

        if (xKeyidx > KEY_Modifiers && xKeyidx < KEY_Modifiers_end) { /* Is this a modifier key? */
            _modifiers &= ~(getModifierBit(xKeyidx));

        }else{ // keycode should be added to report
            if(xKeyidx > KEY_extend && xKeyidx < KEY_extend_end){
                xKeyidx = pgm_read_byte(&keycode_USB_extend[xKeyidx - (KEY_extend + 1)]); 
            }

            gIdx = findIndex(reportBuffer, xKeyidx);

            if(gIdx >= 0 ){
                delete(reportBuffer, gIdx);
                _countOfBuffer--;
            }
            
            
        }
    }

    return;
}

static uint8_t pushKeyindexBuffer(uint8_t xKeyidx, bool xIsDown){
   

    if(xIsDown){    // down

        makeReportBuffer(xKeyidx, true);
        makeReportBufferExtra(xKeyidx, true);
        makeReportBufferSystem(xKeyidx, true);

    }else{  // up

        makeReportBuffer(xKeyidx, false);
        makeReportBufferExtra(xKeyidx, false);
        makeReportBufferSystem(xKeyidx, false);
    }

    return 1;    
}

static void clearReportBuffer(void){
    memset(reportBuffer, 0, REPORT_KEYS); 
    _countOfBuffer = 0;
    _modifiers = 0;
    extraData = 0;
}

static void scanKeyUsbWithMacro(void){
    // 멀티미디어 키를 처리하기 전까지 대기;
    if(_extraHasChanged || _systemHasChanged) return;

    scanKeyWithMacro();
}

#if !USB_COUNT_SOF
static void wakeUpUsb(void){
    wakeUp();
}

static void countSleepUsb(void){
    countSleep();
}
#endif    

bool hasUpdateUsb(void){
    return updateNeeded;
}

static interface_update_t updateUsb = {
        hasUpdateUsb         // hasUpdate
};
static keyscan_driver_t driverKeyScanUsb = {
    pushKeyindexBuffer   // pushKeyCodeWhenChange
};

#if USB_COUNT_SOF
static void wake_up_signal(void)
{
    cli();
    char ps_p = P2U_PS2_PORT;
    char ps_ddr = P2U_PS2_DDR;
    PORTD |=P2U_PS2_CLOCK_BIT;
    PORTD &= ~(P2U_PS2_DATA_BIT);
    DDRD |= (P2U_PS2_CLOCK_BIT)|(P2U_PS2_DATA_BIT);
    _delay_ms(100);
    PORTD ^=(P2U_PS2_CLOCK_BIT);
    PORTD ^=(P2U_PS2_DATA_BIT);
    _delay_ms(3);
    P2U_PS2_PORT = ps_p;
    P2U_PS2_DDR = ps_ddr;
    sei();
}
#endif

/**
 * Main function, containing the main loop that manages timer- and
 * USB-functionality.
 * /return the obligatory integer that nobody cares about...
 */
void usb_main(void) {

    // USB Reset by device only required on Watchdog Reset         
    P2U_PS2_PORT &= ~((1 << P2U_PS2_CLOCK_PIN)|(1 << P2U_PS2_DATA_PIN)); // input:tri-state     output:low                 
    P2U_USB_CFG_DDR &= ~((1 << P2U_USB_CFG_DPLUS_BIT)|(1 << P2U_USB_CFG_DMINUS_BIT));// input, remove USB reset condition

    usbInit();

    int interfaceCount = 0;

    if( eeprom_read_byte((uint8_t *)EEPROM_USB_COMPATIBILITY) == 0x00 )
    {
        usbDeviceDisconnect();  /* do this while interrupts are disabled */
        uchar   i = 0;
        do{             /* fake USB disconnect for > 250 ms */
            wdt_reset();
            _delay_ms(1);
        }while(--i);
    }

    usbDeviceConnect();

    // init
    setKeyScanDriver(&driverKeyScanUsb);
    setUpdateDriver(&updateUsb);
    clearReportBuffer();

    sei();

#if USB_COUNT_SOF    
    bool _isSuspended = false;
    int suspendCount = 0;
#endif

    for(;;){

        /*
         * USB3.0 + win10의 경우 usbDeviceConnect() 딜레이가 있으면 일부 장치에서 인식이 되지않아 이를 제거
         * 하지만, 구식 부트로더 (ps2avrGB_bootloader_140623)의 경우 빠져나올 때 usbDeviceDisconnect()를 처리하지 않아서
         * 이를 여기서 처리해줌.
         */
        if(interfaceReady == false && interfaceCount++ > 1000)
        {
            cli();

            usbDeviceDisconnect();
            wdt_enable(WDTO_15MS);
            for(;;);
        }

#if USB_COUNT_SOF
        if (usbSofCount != 0) {
            _isSuspended = false;
            usbSofCount = 0;
            suspendCount = 0;
            _usbReset = false;

            wakeUp();

        } else if(_usbReset == true){
            _isSuspended = false;
            usbSofCount = 1;

            wakeUp();

        }else{
            // Suspend when no SOF in 3ms-10ms(7.1.7.4 Suspending of USB1.1)
            if (_isSuspended == false && suspendCount++ > 1000 && isReleaseAll()) {
                _isSuspended = true;

                sleep();
            }
        }
#endif

        // main event loop
        usbPoll();

        // if an update is needed, send the report
        if (usbInterruptIsReady()) {

            scanKeyUsbWithMacro(); // changes?

            if(updateNeeded){
                if(interfaceReady==false) continue;

#if USB_COUNT_SOF
                if(_isSuspended == true) {
                    wake_up_signal();
                    updateNeeded = 0;
                    continue;
                }
#endif

                memset(reportKeyboard, 0, REPORT_SIZE_KEYBOARD);

                reportKeyboard[0] = _modifiers;
                reportKeyboard[1] = 0;
                memcpy ( reportKeyboard+2, reportBuffer, _countOfBuffer ); //strlen((char *)reportBuffer) );
//                DBG1(0x06, (uchar *)&reportKeyboard, 8);

                usbSetInterrupt((void *)&reportKeyboard, sizeof(reportKeyboard));
                updateNeeded = 0;

#if !USB_COUNT_SOF
                wakeUpUsb();
#endif

            }else if(_initState == INIT_INDEX_SET_IDLE){
//            	DBG1(0x99, (uchar *)&_initState, 1);

                _initState = INIT_INDEX_INITED;

                // 재부팅시 첫키 입력 오류를 방지하기 위해서 HID init 후 all release 전송;
                memset(reportKeyboard, 0, REPORT_SIZE_KEYBOARD);
                usbSetInterrupt((void *)&reportKeyboard, sizeof(reportKeyboard));
                clearMatrix();

#if !USB_COUNT_SOF
                wakeUpUsb();
#endif

                // 플러깅 후 출력되는 메세지는 넘락등 LED가 반응한 후에 보여진다.
                // usbInterruptIsReady() 일지라도 LED 반응 전에는 출력이 되지 않는다.
                // LED 반응 후에 처리하려고 하면 MAC OS에서 실행되지 않는다.
                // (MAC OS에서는 플러깅 시 LED가 반응하지 않는다. 대신 바로 출력이 된다.)
                // for os x

//                DBG1(0xAA, (uchar *)&idleRate, 1);

            }else if(_ledInitState == INIT_LED_INDEX_INITED){
                _ledInitState = INIT_LED_INDEX_COMPLETE;
                // for windows

            }

            // ps2avrU loop, must be after scan matrix;
            enterFrame();

        }

#if !defined( USING_SIMPLE_MODE )
        if(usbInterruptIsReady3()){
            if(_extraHasChanged){
                report_extra_t gReportExtra = {
                    .report_id = REPORT_ID_CONSUMER,
                    .usage = extraData
                };
                usbSetInterrupt3((void *)&gReportExtra, sizeof(gReportExtra));
                _extraHasChanged = false;
#if !USB_COUNT_SOF
                wakeUpUsb();
#endif
            }
            if(_systemHasChanged){
                report_extra_t gReportExtra = {
                    .report_id = REPORT_ID_SYSTEM,
                    .usage = systemData
                };
                usbSetInterrupt3((void *)&gReportExtra, sizeof(gReportExtra));
                _systemHasChanged = false;
#if !USB_COUNT_SOF
                wakeUpUsb();
#endif
            }
        }
#endif

        if(_initState == INIT_INDEX_INITED){
            if(initCount++ == 200){       // delay for OS X USB multi device
                // all platform init led
                initAfterInterfaceMount();
//                DBG1(0xAB, (uchar *)&initCount, 2);
            }else if(initCount > 200){
                initCount = 201;
                _initState = INIT_INDEX_COMPLETE;
            }
        }

#if !USB_COUNT_SOF
        // 입력이 한동안 없으면 슬립모드로;
        countSleepUsb();
#endif

    }

/*
#if !defined(INTERFACE_ONLY_USB)
    // data line reset;
    USB_INTR_ENABLE &= ~(1 << USB_INTR_ENABLE_BIT);
#endif
*/

}
