/**
 * \file firmware/usbconfig.h
 * \brief Configuration options for the USB-driver.
 *
 * This file is almost identical to the original usbconfig.h by Christian
 * Starkjohann, in structure and content.
 *
 * It contains parts of the USB driver which can be configured and can or must
 * be adapted to your hardware.
 * \author Ronald Schaten <ronald@schatenseite.de>
 * \version $Id: usbconfig.h,v 1.1 2008-07-09 20:47:12 rschaten Exp $
 *
 * License: GNU GPL v2 (see License.txt)
 */

#ifndef __usbconfig_h_included__
#define __usbconfig_h_included__

/* ---------------------------- Hardware Config ---------------------------- */

/** This is the port where the USB bus is connected. When you configure it to
 * "B", the registers PORTB, PINB and DDRB will be used.
 */
#define USB_CFG_IOPORTNAME      D
/** This is the bit number in USB_CFG_IOPORT where the USB D- line is connected.
 * This may be any bit in the port.
 */
#define USB_CFG_DMINUS_BIT      3
/** This is the bit number in USB_CFG_IOPORT where the USB D+ line is connected.
 * This may be any bit in the port. Please note that D+ must also be connected
 * to interrupt pin INT0!
 */
#define USB_CFG_DPLUS_BIT       2

/* ----------------------- Optional Hardware Config ------------------------ */

/* If you connect the 1.5k pullup resistor from D- to a port pin instead of
 * V+, you can connect and disconnect the device from firmware by calling
 * the macros usbDeviceConnect() and usbDeviceDisconnect() (see usbdrv.h).
 * This constant defines the port on which the pullup resistor is connected.
 */
#define USB_CFG_PULLUP_IOPORTNAME   D 
/* This constant defines the bit number in USB_CFG_PULLUP_IOPORT (defined
 * above) where the 1.5k pullup resistor is connected. See description
 * above for details.
 */
#define USB_CFG_PULLUP_BIT          3 

/* --------------------------- Functional Range ---------------------------- */

/** Define this to 1 if you want to compile a version with two endpoints: The
 * default control endpoint 0 and an interrupt-in endpoint 1.
 */
#define USB_CFG_HAVE_INTRIN_ENDPOINT    1
/** Define this to 1 if you want to compile a version with three endpoints: The
 * default control endpoint 0, an interrupt-in endpoint 1 and an interrupt-in
 * endpoint 3. You must also enable endpoint 1 above.
 */
#define USB_CFG_HAVE_INTRIN_ENDPOINT3   0
/** Define this to 1 if you also want to implement the ENDPOINT_HALT feature
 * for endpoint 1 (interrupt endpoint). Although you may not need this feature,
 * it is required by the standard. We have made it a config option because it
 * bloats the code considerably.
 */
#define USB_CFG_IMPLEMENT_HALT          0
/** If you compile a version with endpoint 1 (interrupt-in), this is the poll
 * interval. The value is in milliseconds and must not be less than 10 ms for
 * low speed devices.
 */
#define USB_CFG_INTR_POLL_INTERVAL      10
/** Define this to 1 if the device has its own power supply. Set it to 0 if the
 * device is powered from the USB bus.
 */
#define USB_CFG_IS_SELF_POWERED         0
/** Set this variable to the maximum USB bus power consumption of your device.
 * The value is in milliamperes. [It will be divided by two since USB
 * communicates power requirements in units of 2 mA.]
 */
#define USB_CFG_MAX_BUS_POWER           100
/** Set this to 1 if you want usbFunctionWrite() to be called for control-out
 * transfers. Set it to 0 if you don't need it and want to save a couple of
 * bytes.
 */
#define USB_CFG_IMPLEMENT_FN_WRITE      1
/** Set this to 1 if you need to send control replies which are generated
 * "on the fly" when usbFunctionRead() is called. If you only want to send
 * data from a static buffer, set it to 0 and return the data from
 * usbFunctionSetup(). This saves a couple of bytes.
 */
#define USB_CFG_IMPLEMENT_FN_READ       0
/** Define this to 1 if you want to use interrupt-out (or bulk out) endpoint 1.
 * You must implement the function usbFunctionWriteOut() which receives all
 * interrupt/bulk data sent to endpoint 1.
 */
#define USB_CFG_IMPLEMENT_FN_WRITEOUT   0
/** Define this to 1 if you want flowcontrol over USB data. See the definition
 * of the macros usbDisableAllRequests() and usbEnableAllRequests() in
 * usbdrv.h.
 */
#define USB_CFG_HAVE_FLOWCONTROL        0

/* -------------------------- Device Description --------------------------- */

/** We cannot use Obdev's free shared VID/PID pair because this is a HID.
 * We use John Hyde's VID (author of the book "USB Design By Example") for
 * this example instead. John has offered this VID for use by students for
 * non-commercial devices. Well... This example is for demonstration and
 * education only... DO NOT LET DEVICES WITH THIS VID ESCAPE YOUR LAB!
 * The Product-ID is a random number.
 *
 * USB vendor ID for the device, low byte first. If you have registered your
 * own Vendor ID, define it here. Otherwise you use obdev's free shared
 * VID/PID pair. Be sure to read USBID-License.txt for rules!
 */
#define  USB_CFG_VENDOR_ID       0x42, 0x42
/** This is the ID of the product, low byte first. It is interpreted in the
 * scope of the vendor ID. If you have registered your own VID with usb.org
 * or if you have licensed a PID from somebody else, define it here. Otherwise
 * you use obdev's free shared VID/PID pair. Be sure to read the rules in
 * USBID-License.txt!
 */
#define  USB_CFG_DEVICE_ID       0x31, 0xe1
/** Version number of the device: Minor number first, then major number.
 */
#define USB_CFG_DEVICE_VERSION  0x00, 0x01
/** These two values define the vendor name returned by the USB device. The name
 * must be given as a list of characters under single quotes. The characters
 * are interpreted as Unicode (UTF-16) entities.
 * If you don't want a vendor name string, undefine these macros.
 * ALWAYS define a vendor name containing your Internet domain name if you use
 * obdev's free shared VID/PID pair. See the file USBID-License.txt for
 * details. 
 */
#define USB_CFG_VENDOR_NAME     'w', 'i', 'n', 'k', 'e', 'y', 'l', 'e', 's', 's', '.', 'k', 'r'
/** Length of USB_CFG_DEVICE_VERSION
 */
#define USB_CFG_VENDOR_NAME_LEN 13
/** Same as above for the device name. If you don't want a device name, undefine
 * the macros. See the file USBID-License.txt before you assign a name.
 */
#define USB_CFG_DEVICE_NAME     'p', 's', '2', 'a', 'v', 'r', 'U'
/** Length of USB_CFG_DEVICE_NAME
 */
#define USB_CFG_DEVICE_NAME_LEN 7
/** See USB specification if you want to conform to an existing device class.
 * This setting means to specify the class at the interface level.
 */
#define USB_CFG_DEVICE_CLASS    0
/** See USB specification if you want to conform to an existing device subclass.
 */
#define USB_CFG_DEVICE_SUBCLASS 0
/** See USB specification if you want to conform to an existing device class or
 * protocol. This is HID class.
 */
#define USB_CFG_INTERFACE_CLASS     0x03
/** See USB specification if you want to conform to an existing device class or
 * protocol. This is a boot device.
 */
#define USB_CFG_INTERFACE_SUBCLASS  0x01
/** See USB specification if you want to conform to an existing device class or
 * protocol. This is keyboard protocol.
 */
#define USB_CFG_INTERFACE_PROTOCOL  0x01
/** Define this to the length of the HID report descriptor, if you implement
 * an HID device. Otherwise don't define it or define it to 0.
 */
#define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH    113 //63	//

/* ------------------- Fine Control over USB Descriptors ------------------- */
/* If you don't want to use the driver's default USB descriptors, you can
 * provide our own. These can be provided as (1) fixed length static data in
 * flash memory, (2) fixed length static data in RAM or (3) dynamically at
 * runtime in the function usbFunctionDescriptor(). See usbdrv.h for more
 * information about this function.
 * Descriptor handling is configured through the descriptor's properties. If
 * no properties are defined or if they are 0, the default descriptor is used.
 * Possible properties are:
 *   + USB_PROP_IS_DYNAMIC: The data for the descriptor should be fetched
 *     at runtime via usbFunctionDescriptor().
 *   + USB_PROP_IS_RAM: The data returned by usbFunctionDescriptor() or found
 *     in static memory is in RAM, not in flash memory.
 *   + USB_PROP_LENGTH(len): If the data is in static memory (RAM or flash),
 *     the driver must know the descriptor's length. The descriptor itself is
 *     found at the address of a well known identifier (see below).
 * List of static descriptor names (must be declared PROGMEM if in flash):
 *   char usbDescriptorDevice[];
 *   char usbDescriptorConfiguration[];
 *   char usbDescriptorHidReport[];
 *   char usbDescriptorString0[];
 *   int usbDescriptorStringVendor[];
 *   int usbDescriptorStringDevice[];
 *   int usbDescriptorStringSerialNumber[];
 * Other descriptors can't be provided statically, they must be provided
 * dynamically at runtime.
 *
 * Descriptor properties are or-ed or added together, e.g.:
 * #define USB_CFG_DESCR_PROPS_DEVICE   (USB_PROP_IS_RAM | USB_PROP_LENGTH(18))
 *
 * The following descriptors are defined:
 *   USB_CFG_DESCR_PROPS_DEVICE
 *   USB_CFG_DESCR_PROPS_CONFIGURATION
 *   USB_CFG_DESCR_PROPS_STRINGS
 *   USB_CFG_DESCR_PROPS_STRING_0
 *   USB_CFG_DESCR_PROPS_STRING_VENDOR
 *   USB_CFG_DESCR_PROPS_STRING_PRODUCT
 *   USB_CFG_DESCR_PROPS_STRING_SERIAL_NUMBER
 *   USB_CFG_DESCR_PROPS_HID
 *   USB_CFG_DESCR_PROPS_HID_REPORT
 *   USB_CFG_DESCR_PROPS_UNKNOWN (for all descriptors not handled by the driver)
 *
 */

#define USB_CFG_DESCR_PROPS_DEVICE                  0
#define USB_CFG_DESCR_PROPS_CONFIGURATION           0
#define USB_CFG_DESCR_PROPS_STRINGS                 0
#define USB_CFG_DESCR_PROPS_STRING_0                0
#define USB_CFG_DESCR_PROPS_STRING_VENDOR           0
#define USB_CFG_DESCR_PROPS_STRING_PRODUCT          0
#define USB_CFG_DESCR_PROPS_STRING_SERIAL_NUMBER    0
#define USB_CFG_DESCR_PROPS_HID                     0
#define USB_CFG_DESCR_PROPS_HID_REPORT              0
#define USB_CFG_DESCR_PROPS_UNKNOWN                 0

#endif /* __usbconfig_h_included__ */
