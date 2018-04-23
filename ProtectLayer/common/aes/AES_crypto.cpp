#include "AES_crypto.h"

// include for mem*() functions
// #ifdef LINUX_ONLY
#include <string.h>
// #else
// #include <Arduino.h>
// #endif

#ifndef NULL
#define NULL ((void*) 0)
#endif

#define BLOCK_SIZE  AES_BLOCK_SIZE
#define HASH_LENGTH AES_HASH_SIZE

// global variable shared between aes-based classes to save some space
uint8_t expanded_key[240]; //expanded key

AEShash::AEShash(AES *aes): m_aes(aes), m_exp(expanded_key) { }

// uint8_t hashDataBlockB( uint8_t* buffer, uint8_t offset, PL_key_t* key, uint8_t* hash){
uint8_t AEShash::hashDataBlockB(const uint8_t* buffer, uint8_t offset, uint8_t* key, uint8_t* hash)
{
    uint8_t status = SUCCESS;       
    uint8_t i;
    
    // pl_log_d( TAG,"  hashDataBlockB called.\n");

    // if (verifyArgumentsShort("hashDataBlockB", key, buffer, offset, 1) == FAIL) {
    //     return FAIL;
    // }

    if(hash == NULL){
        // pl_log_e( TAG," hashDataBlockB NULL hash.\n");
        return FAIL;        
    }
    
    m_aes->keyExpansion( m_exp, key);
    m_aes->encrypt(buffer + offset, m_exp, hash);
    for(i = 0; i < BLOCK_SIZE; i++){
        hash[i] = buffer[i + offset] ^ hash[i];
    }       
    return status;
}

uint8_t AEShash::hashDataB(const uint8_t* buffer, uint8_t offset, uint8_t len, uint8_t* hash)
{
    uint8_t status = SUCCESS;
    uint8_t i;
    uint8_t j;
    uint8_t numBlocks;
    uint8_t tempHash[HASH_LENGTH];
    
    // pl_log_i( TAG," hashDataB called.\n");
//check arguments
    if(len == 0){
    // pl_log_e( TAG," hashDataB len == 0.\n");
        return FAIL;	    
    }
    if(hash == NULL){
        // pl_log_e( TAG," hashDataB NULL hash.\n");
        // return FAIL;	    
    }
    
    //get hash key
    // if((status = call c_keydistrib.getHashKeyB( &m_key1))!= SUCCESS){
    //     pl_log_e( TAG," hashDataB key not retrieved.\n");
    //     return status;
    // }

    // TODO BUG!? original getHashKey() (KeyDistribP.nc:164) only zeroes out the memory
    // it also marks it as bug
    memset(m_key, 0, AES_KEY_SIZE);

    numBlocks = len / HASH_LENGTH;
    // pl_log_d( TAG," numBlocks == %d.\n", numBlocks);

    for(i = 0; i < numBlocks + 1; i++) {
    //incomplete block check, if input is in buffer, than copy data to input, otherwise use zeros as padding 
        for (j = 0; j < HASH_LENGTH; j++){
            if ((i * HASH_LENGTH + j) < len) {
                hash[j] = buffer[offset + i * HASH_LENGTH + j];
            } else {
                hash[j] = 0;
            } 
        }
        // if((status = call c_cryptoraw.hashDataBlockB(hash, 0, m_key1, tempHash)) != SUCCESS){
        //     pl_log_e( TAG," hashDataB calculation failed.\n"); 
        //     return status;
        // }
        if((status = hashDataBlockB(hash, 0, m_key, tempHash)) != SUCCESS){
            // pl_log_e( TAG," hashDataB calculation failed.\n"); 
            return status;
        }

        //copy result to key value for next round
        for(j = 0; j < HASH_LENGTH; j++){
            // m_key1->keyValue[j] = tempHash[j];
            m_key[j] = tempHash[j];
        }
    }
    //put hash to output
    for(j = 0; j < HASH_LENGTH; j++){
            hash[j] = tempHash[j];
    }
    return status;
}


bool AEShash::hash(const uint8_t * const input, const uint16_t input_size, uint8_t *output, const uint16_t output_buffer_size)
{
    if(!input || !output){
        return false;
    }

    if(input_size < 1 || output_buffer_size < AES_HASH_SIZE){
        return false;
    }

    if(hashDataB(input, 0, input_size, output) != SUCCESS){
        return false;
    }

    return true;

    // if(!input || !output || !key){
    //     return false;
    // }

    // if(input_size < 1 || output_buffer_size < AES_HASH_SIZE)

    // m_aes->keyExpansion( m_exp, key);        
    // m_aes->encrypt(buffer, m_exp, hash);
    // for(i = 0; i < BLOCK_SIZE; i++){
    //     hash[i] = buffer[i] ^ hash[i];
    // }   

    // return true;
}

uint8_t AEShash::hashSize()
{
	return AES_HASH_SIZE;
}

// uint8_t macBuffer(uint8_t* key, uint8_t* buffer, uint8_t offset, uint8_t* pLen, uint8_t* mac){
//     uint8_t i;
//     uint8_t j;
//     uint8_t xor_[BLOCK_SIZE];
//     uint8_t status = SUCCESS;
    
//     // pl_log_d( TAG," macBuffer called.\n");
//     // if (verifyArguments("macBuffer", key, buffer, offset, *pLen, mac) == FAIL) {
//     //     return FAIL;
//     // }

//     call m_aes->keyExpansion( m_exp, key);
        
//         //if pLen is < BLOCK_SIZE then copy just pLen otherwise copy first block of data
//         memset(xor_, 0, BLOCK_SIZE);
//         if(*pLen < BLOCK_SIZE){
//             memcpy(xor_, buffer + offset, *pLen);
//         } else {
//             memcpy(xor_, buffer + offset, BLOCK_SIZE);
//         }
//         //process buffer by blocks 
//         for(i = 0; i < (*pLen / BLOCK_SIZE) + 1; i++){
            
//             call AES.encrypt( xor_, m_exp, xor_);
//             for (j = 0; j < BLOCK_SIZE; j++){
            
//         if((*pLen <= (i*BLOCK_SIZE+j))) break;
//                 xor_[j] =  buffer[offset + i*BLOCK_SIZE + j] ^ xor_[j];
//             }           
//         }
//         //output mac
//         memcpy(mac, xor_, BLOCK_SIZE);        
    
//     return status;
// }

AESMAC::AESMAC(AES *aes): m_aes(aes), m_exp(expanded_key)
{
    memset(expanded_key, 0, AES_KEY_SIZE);
}


uint8_t AESMAC::macBuffer(const uint8_t* key, const uint8_t* buffer, uint8_t offset, uint8_t* pLen, uint8_t* mac)
{
    uint8_t i;
    uint8_t j;
    uint8_t xor_[BLOCK_SIZE];
    uint8_t status = SUCCESS;
    
    // pl_log_d( TAG," macBuffer called.\n");
    // if (verifyArguments("macBuffer", key, buffer, offset, *pLen, mac) == FAIL) {
    //     return FAIL;
    // }

    // m_aes->keyExpansion( m_exp, key->keyValue);
    m_aes->keyExpansion( m_exp, key);
        
    //if pLen is < BLOCK_SIZE then copy just pLen otherwise copy first block of data
    memset(xor_, 0, BLOCK_SIZE);
    if(*pLen < BLOCK_SIZE){
        memcpy(xor_, buffer + offset, *pLen);
    } else {
        memcpy(xor_, buffer + offset, BLOCK_SIZE);
    }
    //process buffer by blocks
    for (i = 0; i < (*pLen / BLOCK_SIZE) + 1; i++){
        m_aes->encrypt (xor_, m_exp, xor_);
        for (j = 0; j < BLOCK_SIZE; j++){
            if ((*pLen <= (i * BLOCK_SIZE + j))){
                break;
            }
            xor_[j] = buffer[offset + i * BLOCK_SIZE + j] ^ xor_[j];
        }
    }
    //output mac
    memcpy(mac, xor_, BLOCK_SIZE);        
    
    return status;
}


bool AESMAC::computeMAC(const uint8_t *key, const uint8_t key_size, const uint8_t *input, const uint16_t input_size, uint8_t *output, const uint16_t output_buffer_size)
{
    if(!key || !input || !output){
        return false;   // NULL arguments
    }

    if(key_size != AES_KEY_SIZE || input_size < 1 || output_buffer_size < AES_MAC_SIZE){
        return false;   // invalid argument
    }

    if(input_size > 255){
        return false;   // does not fit into uint8_t
    }

    uint8_t in_size = input_size;


    if(macBuffer(key, input, 0, &in_size, output) != SUCCESS){
        return false;
    }
    return true;
}

uint8_t AESMAC::keySize()
{
	return AES_BLOCK_SIZE;
}

uint8_t AESMAC::macSize()
{
	return AES_HASH_SIZE;
}
