#ifndef ADDRESSES_H
#define ADDRESSES_H

#include <stdint.h>

// TODO! use correct size from the rest of the library
#ifndef KEY_SIZE
#define KEY_SIZE 16
#endif


#define CONFIG_START_ADDRESS    (uint8_t*)0x40

#define KEY_STRUCT_SIZE         sizeof(PL_key_t)


// addresses
void* getIDAddr()
{
    return (void*) 0;
}

void* getBSKeyAddr()
{
    return (void*) CONFIG_START_ADDRESS;
}

void* getNodeKeyAddr(uint8_t node_id)
{
    uint8_t* addr = CONFIG_START_ADDRESS + KEY_SIZE;
    for(int i=0;i<node_id - 1/*node 0 does not exist*/;i++){
        addr += KEY_SIZE;
    }

    return addr;
}

#endif //  ADDRESSES_H