#ifndef VERSION_H
#define VERSION_H

#define FIRMWARE_GB             0   // 0=ps2avrGB, 1=ps2avrGB4U
#define FIRMWARE_GB4U           1
#define FIRMWARE_GB_SPLIT       2
#define FIRMWARE_GB4U_SPLIT     3

#define HAS_RGB_LED     (FIRMWARE == FIRMWARE_GB || FIRMWARE == FIRMWARE_GB_SPLIT)
#define HAS_OPTIONS     (FIRMWARE == FIRMWARE_GB || FIRMWARE == FIRMWARE_GB_SPLIT)

// Ver
#define VERSION_MAJOR   1
#define VERSION_MINOR   3
#define VERSION_PATCH   0

#endif
