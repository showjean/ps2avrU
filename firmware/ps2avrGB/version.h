#ifndef VERSION_H
#define VERSION_H

#define FIRMWARE_GB             0
#define FIRMWARE_GB4U           1
#define FIRMWARE_GB_SPLIT       2
#define FIRMWARE_GB4U_SPLIT     3
#define FIRMWARE_GB4U_A         4

#define HAS_RGB_LED     (FIRMWARE == FIRMWARE_GB || FIRMWARE == FIRMWARE_GB_SPLIT)
#define HAS_ROW_17     (FIRMWARE == FIRMWARE_GB4U || FIRMWARE == FIRMWARE_GB4U_SPLIT)
#define HAS_ROW_18     (FIRMWARE == FIRMWARE_GB4U_A)

// Ver
#define VERSION_MAJOR   1
#define VERSION_MINOR   4
#define VERSION_PATCH   0

#endif
