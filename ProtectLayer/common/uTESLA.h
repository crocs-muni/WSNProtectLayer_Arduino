#ifndef UTESLA_H
#define UTESLA_H

#include "../common.h"

#define MSG_TYPE_DATA   3
#define MSG_TYPE_KEY    4

//typedef int8_t error_t; //TODO  defined in errno.h as int
typedef int error_t; //TODO  defined in errno.h as int
#define FAIL            1
#define SUCCESS         0


class Hash {
public:
    virtual bool hash(const uint8_t * const input, const uint16_t input_size, uint8_t *output, const uint16_t output_buufer_size) = 0;
    virtual uint8_t hashSize() = 0;
};

class MAC {
public:
    virtual bool computeMAC(const uint8_t *key, const uint8_t key_size, const uint8_t *input, const uint16_t input_size, uint8_t *output, const uint16_t output_buffer_size) = 0;
    virtual uint8_t keySize() = 0;
    virtual uint8_t macSize() = 0;
};

#endif //  UTESLA_H
