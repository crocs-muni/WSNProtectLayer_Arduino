#ifndef ADDRESSES_H
#define ADDRESSES_H

#include <stdint.h>

// TODO! use correct size from the rest of the library
#ifndef KEY_SIZE
#define KEY_SIZE 16
#endif


#define CONFIG_START_ADDRESS    (uint8_t*)0x40

#define KEY_STRUCT_SIZE         sizeof(PL_key_t)

#endif //  ADDRESSES_H