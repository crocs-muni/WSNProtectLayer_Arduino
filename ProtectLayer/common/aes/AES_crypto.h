#ifndef AESCRYPTO_H
#define AESCRYPTO_H

#include <stdint.h>

#define KEY_SIZE        16

#include "uTESLA.h"
#include "AES.h"


class AEShash: public Hash {
private:
    uint8_t m_key[AES_BLOCK_SIZE] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                     0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    AES     *m_aes;
    // uint8_t m_exp[240]; //expanded key
    uint8_t *m_exp; //expanded key

    uint8_t hashDataBlockB(const uint8_t* buffer, uint8_t offset, uint8_t* key, uint8_t* hash);
    uint8_t hashDataB(const uint8_t* buffer, uint8_t offset, uint8_t len, uint8_t* hash);
public:
    AEShash(AES *aes);
    virtual bool hash(const uint8_t * const input, const uint16_t input_size, uint8_t *output, const uint16_t output_buufer_size);
    virtual uint8_t hashSize();
};

class AESMAC: public MAC {
private:
    AES *m_aes;
    uint8_t *m_exp; //expanded key

    uint8_t macBuffer(const uint8_t* key, const uint8_t* buffer, uint8_t offset, uint8_t* pLen, uint8_t* mac);
public:
    AESMAC(AES *aes);
    virtual bool computeMAC(const uint8_t *key, const uint8_t key_size, const uint8_t *input, const uint16_t input_size, uint8_t *output, const uint16_t output_buffer_size);
    virtual uint8_t keySize();
    virtual uint8_t macSize();
};

class AEScipher { // TODO abstract class? 
    AES *m_aes;

public:
    // no abstract class for cipher - not virtual yet
    bool encrypt(const uint8_t *key, const uint8_t key_size /**/, const uint8_t *input, const uint16_t input_size, uint8_t *output, const uint8_t output_buffer_size);
    uint8_t keySize();
    uint8_t blockSize(); // TODO remove?
};

#endif //  AESCRYPTO_H
