/**
 * @brief AES-based classes for cryptographic operations
 * 
 * @file    AES_crypto.h
 * @author  Martin Sarkany
 * @date    05/2018
 */

#ifndef AESCRYPTO_H
#define AESCRYPTO_H

#include <stdint.h>

#include "AES.h"
#include "common.h"

/**
 * @brief Class for hash computation
 * 
 */
class AEShash: public Hash {
private:
    AES     *m_aes; // AES class used for AES encryption
    uint8_t *m_exp; //  expanded key

public:
    /**
     * @brief Constructor
     * 
     * @param aes AES class
     */
    AEShash(AES *aes);

    /**
     * @brief Hash single block
     * 
     * @param buffer    Data to be hashed
     * @param offset    Offset to skip
     * @param key       Key to use for AES
     * @param hash      Output
     * @return uint8_t  SUCCESS or FAIL
     */
    virtual uint8_t hashDataBlockB(const uint8_t* buffer, uint8_t offset, uint8_t* key, uint8_t* hash);

    /**
     * @brief Hash data
     * 
     * @param buffer    Data to be hashed
     * @param offset    Offset to skip
     * @param len       Size of the data
     * @param hash      Output
     * @return uint8_t  SUCCESS or FAIL
     */
    virtual uint8_t hashDataB(const uint8_t* buffer, uint8_t offset, uint8_t len, uint8_t* hash);
    
    /**
     * @brief Hash data
     * 
     * @param input                 Input
     * @param input_size            Input size
     * @param output                Output buffer
     * @param output_buffer_size    Output buffer size
     * @return true                 Success
     * @return false                Failure
     */
    virtual bool hash(const uint8_t * const input, const uint16_t input_size, uint8_t *output, const uint16_t output_buffer_size);

    /**
     * @brief Get size of the output
     * 
     * @return uint8_t Size of the output
     */
    virtual uint8_t hashSize();
};


/**
 * @brief Class for MAC computation
 * 
 */
class AESMAC: public MAC {
private:
    AES *m_aes;     // AES class used for AES encryption
    uint8_t *m_exp; // expanded key

public:
    /**
     * @brief Constructor
     * 
     * @param aes   AES class
     */
    AESMAC(AES *aes);

    /**
     * @brief Compute MAC for buffer
     * 
     * @param key       Key for AES encryption
     * @param buffer    Data to be hashed
     * @param offset    Offset to skip
     * @param pLen      Size of the data
     * @param mac       Output
     * @return uint8_t  SUCCESS or FAIL
     */
    virtual uint8_t macBuffer(const uint8_t* key, const uint8_t* buffer, uint8_t offset, uint8_t* pLen, uint8_t* mac);

    /**
     * @brief Compute MAC for buffer
     * 
     * @param key                   Key for AES encryption
     * @param key_size              Size of the key
     * @param input                 Input
     * @param input_size            Input size
     * @param output                Output
     * @param output_buffer_size    Output buffer size
     * @return true                 Success
     * @return false                Failure
     */
    virtual bool computeMAC(const uint8_t *key, const uint8_t key_size, const uint8_t *input, const uint16_t input_size, uint8_t *output, const uint16_t output_buffer_size);
    
    /**
     * @brief Get size of the key
     * 
     * @return uint8_t Size of the key
     */
    virtual uint8_t keySize();

    /**
     * @brief Get MAC size
     * 
     * @return uint8_t MAC size
     */
    virtual uint8_t macSize();
};

#endif //  AESCRYPTO_H
