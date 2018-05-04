/**
 * @brief AES-based cryptographic operations. Parts of the source code were taken from WSNProtectLayer (https://github.com/crocs-muni/WSNProtectLayer)
 * 
 * @file    AES_crypto.cpp
 * @author  Martin Sarkany
 * @date    05/2018
 */

#include "AES_crypto.h"

#include <string.h>

// global variable shared between aes-based classes to save some space
uint8_t expanded_key[240]; //expanded key

AEShash::AEShash(AES *aes): m_aes(aes), m_exp(expanded_key) { }

// uint8_t hashDataBlockB( uint8_t* buffer, uint8_t offset, PL_key_t* key, uint8_t* hash){
uint8_t AEShash::hashDataBlockB(const uint8_t* buffer, uint8_t offset, uint8_t* key, uint8_t* hash)
{
    uint8_t status = SUCCESS;       
    uint8_t i;
    
    if(hash == NULL){
        return FAIL;        
    }
    
    m_aes->keyExpansion( m_exp, key);
    m_aes->encrypt(buffer + offset, m_exp, hash);
    for(i = 0; i < AES_BLOCK_SIZE; i++){
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
    uint8_t tempHash[AES_HASH_SIZE];
    uint8_t key_buff[AES_BLOCK_SIZE];
    
    //check arguments
    if(len == 0){
        return FAIL;	    
    }
    if(hash == NULL){
        return FAIL;	    
    }

    // TODO BUG!? original getHashKey() (KeyDistribP.nc:164) only zeroes out the memory
    // it also marks it as bug
    memset(key_buff, 0, AES_KEY_SIZE);

    numBlocks = len / AES_HASH_SIZE;
    
    for(i = 0; i < numBlocks + 1; i++) {
    //incomplete block check, if input is in buffer, than copy data to input, otherwise use zeros as padding 
        for (j = 0; j < AES_HASH_SIZE; j++){
            if ((i * AES_HASH_SIZE + j) < len) {
                hash[j] = buffer[offset + i * AES_HASH_SIZE + j];
            } else {
                hash[j] = 0;
            } 
        }
        if((status = hashDataBlockB(hash, 0, key_buff, tempHash)) != SUCCESS){
            return status;
        }

        //copy result to key value for next round
        for(j = 0; j < AES_HASH_SIZE; j++){
            key_buff[j] = tempHash[j];
        }
    }
    
    //put hash to output
    for(j = 0; j < AES_HASH_SIZE; j++){
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
}

uint8_t AEShash::hashSize()
{
	return AES_HASH_SIZE;
}

AESMAC::AESMAC(AES *aes): m_aes(aes), m_exp(expanded_key)
{
    memset(expanded_key, 0, AES_KEY_SIZE);
}


uint8_t AESMAC::macBuffer(const uint8_t* key, const uint8_t* buffer, uint8_t offset, uint8_t* pLen, uint8_t* mac)
{
    uint8_t i;
    uint8_t j;
    uint8_t xor_[AES_BLOCK_SIZE];
    uint8_t status = SUCCESS;
    
    m_aes->keyExpansion( m_exp, key);
        
    //if pLen is < BLOCK_SIZE then copy just pLen otherwise copy first block of data
    memset(xor_, 0, AES_BLOCK_SIZE);
    if(*pLen < AES_BLOCK_SIZE){
        memcpy(xor_, buffer + offset, *pLen);
    } else {
        memcpy(xor_, buffer + offset, AES_BLOCK_SIZE);
    }
    //process buffer by blocks
    for (i = 0; i < (*pLen / AES_BLOCK_SIZE) + 1; i++){
        m_aes->encrypt (xor_, m_exp, xor_);
        for (j = 0; j < AES_BLOCK_SIZE; j++){
            if ((*pLen <= (i * AES_BLOCK_SIZE + j))){
                break;
            }
            xor_[j] = buffer[offset + i * AES_BLOCK_SIZE + j] ^ xor_[j];
        }
    }
    //output mac
    memcpy(mac, xor_, AES_BLOCK_SIZE);        
    
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
