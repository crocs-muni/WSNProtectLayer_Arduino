#ifndef ADDRESSES_H
#define ADDRESSES_H

#include <stdint.h>

// TODO! use correct size from the rest of the library
#ifndef KEY_SIZE
#define KEY_SIZE 16
#endif


#define CONFIG_START_ADDRESS    (uint8_t*)0x40
#define UTESLA_KEY_ADDRESS      (uint8_t*)0x14F

#endif //  ADDRESSES_H