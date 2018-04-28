#ifndef UTESLA_H
#define UTESLA_H

#include "common.h"

#define MSG_TYPE_DATA   3
#define MSG_TYPE_KEY    4


class Hash {
public:
    virtual uint8_t hashDataBlockB(const uint8_t* buffer, uint8_t offset, uint8_t* key, uint8_t* hash) = 0; // TODO remove
    virtual uint8_t hashDataB(const uint8_t* buffer, uint8_t offset, uint8_t len, uint8_t* hash) = 0;       // TODO remove
    virtual bool hash(const uint8_t * const input, const uint16_t input_size, uint8_t *output, const uint16_t output_buufer_size) = 0;
    virtual uint8_t hashSize() = 0;
};

class MAC {
public:
    virtual uint8_t macBuffer(const uint8_t* key, const uint8_t* buffer, uint8_t offset, uint8_t* pLen, uint8_t* mac) = 0;  // TODO remove
    virtual bool computeMAC(const uint8_t *key, const uint8_t key_size, const uint8_t *input, const uint16_t input_size, uint8_t *output, const uint16_t output_buffer_size) = 0;
    virtual uint8_t keySize() = 0;
    virtual uint8_t macSize() = 0;
};

#endif //  UTESLA_H
