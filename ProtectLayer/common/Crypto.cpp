/**
 * @brief Implementation of cryptographic functions. Most of the source code is taken from WSNProtectLayer (https://github.com/crocs-muni/WSNProtectLayer).
 * Logging has been removed due to memory limitations.
 * 
 * @file    Crypto.cpp
 * @author  Martin Sarkany
 * @date    05/2018
 */

#include "Crypto.h"

#include <string.h>


Crypto::Crypto(Cipher *cipher, MAC *mac, Hash *hash, KeyDistrib *keydistrib):
m_cipher(cipher), m_mac(mac), m_hash(hash), m_keydistrib(keydistrib), m_key1(NULL), m_exp(expanded_key)
{

}


//
//	Crypto interface
//	


uint8_t Crypto::protectBufferForNodeB(node_id_t nodeID, uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;

    if((status = m_keydistrib->getKeyToNodeB(nodeID, &m_key1)) != SUCCESS){
        return status;
    }

    return protectBufferB(m_key1, buffer, offset, pLen);
}

uint8_t Crypto::unprotectBufferFromNodeB(node_id_t nodeID, uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;		
    
    if((status = m_keydistrib->getKeyToNodeB(nodeID, &m_key1)) != SUCCESS){
        return status;
    }

// // TODO! REMOVE
// #ifdef __linux__
//     printf("C%d: %u\n", nodeID, *m_key1->counter);
// #endif 

    return unprotectBufferB(m_key1, buffer, offset, pLen);
}

uint8_t Crypto::protectBufferForBSB(uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;	

    if((status = m_keydistrib->getKeyToBSB(&m_key1))!= SUCCESS){
        return status;
    }
    
    return protectBufferB(m_key1, buffer, offset, pLen);
}

uint8_t Crypto::unprotectBufferFromBSB(uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;		

    if((status = m_keydistrib->getKeyToBSB(&m_key1))!= SUCCESS){
        return status;
    }

    return unprotectBufferB(m_key1, buffer, offset, pLen);
}

uint8_t Crypto::macBufferForNodeB(node_id_t nodeID, uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;

    if((status = m_keydistrib->getKeyToNodeB(nodeID, &m_key1)) == SUCCESS){
        status = macBuffer(m_key1, buffer, offset, pLen, buffer + offset + *pLen);
        *pLen = *pLen + MAC_LENGTH;
    }
    
    return status;
}

uint8_t Crypto::macBufferForBSB(uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;

    if((status =  m_keydistrib->getKeyToBSB(&m_key1)) == SUCCESS){	
        status = macBuffer(m_key1, buffer, offset, pLen, buffer + offset + *pLen);
        *pLen = *pLen + MAC_LENGTH; // TODO MAC_LENGTH => m_mac->macSize() everywhere
    }
    
    return status;       
}

uint8_t Crypto::verifyMacFromNodeB(node_id_t nodeID, uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;        
        
    if((status = m_keydistrib->getKeyToNodeB(nodeID, &m_key1)) != SUCCESS){
        return FAIL;
    }
    
    return verifyMac(m_key1, buffer,  offset, pLen);
}

uint8_t Crypto::verifyMacFromBSB(uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;        
        
    if((status = m_keydistrib->getKeyToBSB(&m_key1)) != SUCCESS){
        return status;
    }

    return verifyMac(m_key1, buffer,  offset, pLen);
}

uint8_t Crypto::hashDataB(uint8_t* buffer, uint8_t offset, uint8_t len, uint8_t* hash)
{
    return m_hash->hashDataB(buffer, offset, len, hash);
}

uint8_t Crypto::hashDataShortB(uint8_t* buffer, uint8_t offset, uint8_t len, uint32_t* hash)
{
    uint8_t status;

    if(hash == NULL){
        return FAIL;	    
    }
    
    if((status = m_hash->hashDataB(buffer, offset, len, reinterpret_cast<uint8_t*>(hash))) != SUCCESS){
        return status;
    }
    
    return SUCCESS;
}

uint8_t Crypto::verifyHashDataB(uint8_t* buffer, uint8_t offset, uint8_t pLen, uint8_t* hash)
{
    uint8_t status = SUCCESS;
    uint8_t tempHash[BLOCK_SIZE];

    if((status = m_hash->hashDataB(buffer, offset, pLen, tempHash)) != SUCCESS){
        return FAIL;
    }

    if(memcmp(tempHash, hash, BLOCK_SIZE) != 0){
        return EWRONGHASH;
    }

    return status;
}

uint8_t Crypto::verifyHashDataShortB(uint8_t* buffer, uint8_t offset, uint8_t pLen, uint32_t hash)
{
    uint8_t status = SUCCESS;
    uint32_t tempHash = 0;

    if((status = hashDataShortB(buffer, offset, pLen, &tempHash)) != SUCCESS){
        return FAIL;
    }
    
    if(tempHash != hash){
        return EWRONGHASH;
    }

    return status;
}

//
//	CryptoRaw interface
//	

uint8_t Crypto::encryptBufferB(PL_key_t* key, uint8_t* buffer, uint8_t offset, uint8_t len)
{
    uint8_t i;
    uint8_t j;
    uint8_t plainCounter[BLOCK_SIZE];			
    uint8_t encCounter[BLOCK_SIZE];

    //set rest of counter to zeros to fit AES block
    memset(plainCounter, 0, BLOCK_SIZE);	
    
    m_cipher->keyExpansion(m_exp, (uint8_t*) key->keyValue);

    //process buffer by blocks 
    for(i = 0; i < (len / BLOCK_SIZE) + 1; i++){        
        plainCounter[0] =  (*key->counter);
        plainCounter[1] =  (*(key->counter)) >> 8;
        plainCounter[2] =  (*(key->counter)) >> 16;
        plainCounter[3] =  (*(key->counter)) >> 24;
        m_cipher->encrypt(plainCounter, m_exp, encCounter);
        
        for (j = 0; j < BLOCK_SIZE; j++){
            if (i*BLOCK_SIZE + j >= len){
                break;
            }
            buffer[offset + i*BLOCK_SIZE + j] ^= encCounter[j];
        }
        (*(key->counter))++;
        if((*(key->counter)) == 0){                        
            //deal with new key and counter value reset
        }
    }
    return SUCCESS;
}

uint8_t Crypto::decryptBufferB(PL_key_t* key, uint8_t* buffer, uint8_t offset, uint8_t len)
{
    //for counter mode encrypt is same as decrypt
    return encryptBufferB(key, buffer, offset, len);
}

uint8_t Crypto::macBuffer(PL_key_t* key, uint8_t* buffer, uint8_t offset, uint8_t* pLen, uint8_t* mac)
{
    return m_mac->macBuffer(key->keyValue, buffer, offset, pLen, mac);
}

uint8_t Crypto::verifyMac(PL_key_t* key, uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t mac[MAC_LENGTH];
    uint8_t macLength = *pLen;
    uint8_t status = SUCCESS;        

    if (*pLen < MAC_LENGTH){
        return FAIL;
    }

    macLength = macLength - MAC_LENGTH;
    status = macBuffer(key, buffer, offset, &macLength, mac); //calculate new mac	

    if((memcmp(mac, buffer + offset + *pLen - MAC_LENGTH, MAC_LENGTH)) != 0){ //compare new with received
        return EWRONGMAC;            
    }

    return status;
}


uint8_t Crypto::deriveKeyB(PL_key_t* masterKey, uint8_t* derivationData, uint8_t offset, uint8_t len, PL_key_t* derivedKey)
{
    if(masterKey == NULL){
        return FAIL;	    
    }

    if(offset > MAX_OFFSET){
        return FAIL;	    
    }

    if(len != BLOCK_SIZE){
        return FAIL;	    
    }       
    
    if(derivationData == NULL){
        return FAIL;	    
    }        
    
    if(derivedKey == NULL){
        return FAIL;	    
    }
    
    m_cipher->keyExpansion(m_exp, (uint8_t*)(masterKey->keyValue));
    m_cipher->encrypt(derivationData + offset, m_exp, (uint8_t*)(derivedKey->keyValue));
    
    return SUCCESS;
}

uint8_t Crypto::hashDataBlockB(uint8_t* buffer, uint8_t offset, PL_key_t* key, uint8_t* hash)
{
    return m_hash->hashDataBlockB(buffer, offset, key->keyValue, hash);
}


uint8_t Crypto::protectBufferB(PL_key_t* key, uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;

    //offset is used for encryption shift, to mac, but not encrypt SPheader
    if((status = macBuffer(key, buffer, 0, pLen, buffer + *pLen)) != SUCCESS){
        return status;
    }

    if((status = encryptBufferB(key, buffer, offset, *pLen)) != SUCCESS){
        return status;		
    }

    *pLen += m_mac->macSize();   // TODO correct?
    
    return status;
}

uint8_t Crypto::unprotectBufferB(PL_key_t* key, uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;
    uint8_t i;
    uint32_t counter = *(key->counter);
    uint8_t buffer_copy[MAX_MSG_SIZE];

    if(*pLen > MAX_MSG_SIZE){
        return FAIL;
    }

    memcpy(buffer_copy, buffer, *pLen);

    if(*pLen < m_mac->macSize()){
        return FAIL;
    }
    
    //offset is used for encryption shift, to verify SPheader, but not to encrypt it
    // if((status = decryptBufferB(key, buffer, offset, *pLen) != SUCCESS){
    if((status = decryptBufferB(key, buffer, offset, *pLen - m_mac->macSize())) != SUCCESS){
        return status;
    }

    if((status = verifyMac(key, buffer, 0, pLen)) != SUCCESS){            
        for (i = 1; i <= COUNTER_SYNCHRONIZATION_WINDOW; i++){    
memcpy(buffer, buffer_copy, *pLen);
            *(key->counter) = counter - i;
            decryptBufferB(key, buffer, offset, *pLen - m_mac->macSize());
            if((status = verifyMac(key, buffer, 0, pLen)) == SUCCESS){
                return status;
            }
    
memcpy(buffer, buffer_copy, *pLen);
            *(key->counter) = counter + i;
            decryptBufferB(key, buffer, offset, *pLen - m_mac->macSize());
            if((status = verifyMac(key, buffer, 0, pLen)) == SUCCESS){
                return status;
            }
        }
        *(key->counter) = counter;
        return status;
    }
    
    return status;
}
