#include "Crypto.h"

#include <string.h>


Crypto::Crypto(Cipher *cipher, MAC *mac, Hash *hash, KeyDistrib *keydistrib):
m_cipher(cipher), m_mac(mac), m_hash(hash), m_keydistrib(keydistrib), m_exp(expanded_key)
{

}

uint8_t Crypto::protectBufferForNodeB(node_id_t nodeID, uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;
    
    // pl_log_i(TAG," protectBufferForNodeB called.\n"); 

    if((status = m_keydistrib->getKeyToNodeB(nodeID, &m_key1)) != SUCCESS){
        // pl_log_e(TAG," protectBufferForNodeB key not retrieved.\n");
        return status;
    }
    
    return protectBufferB(m_key1, buffer, offset, pLen);
}

uint8_t Crypto::unprotectBufferFromNodeB(node_id_t nodeID, uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;		
    
    // pl_log_i(TAG," unprotectBufferFromNodeB called.\n"); 
    
    if((status = m_keydistrib->getKeyToNodeB(nodeID, &m_key1))!= SUCCESS){
        // pl_log_e(TAG," unprotectBufferFromNodeB key not retrieved.\n");
        return status;
    }

    return unprotectBufferB(m_key1, buffer, offset, pLen);
}

uint8_t Crypto::protectBufferForBSB(uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;	

    // pl_log_i(TAG," protectBufferForBSB called.\n"); 

    if((status = m_keydistrib->getKeyToBSB(&m_key1))!= SUCCESS){
        // pl_log_e(TAG," protectBufferForBSB key not retrieved.\n");
        return status;
    }
    
    return protectBufferB(m_key1, buffer, offset, pLen);
}

uint8_t Crypto::unprotectBufferFromBSB(uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;		

    // pl_log_i(TAG," unprotectBufferBSB called.\n"); 

    if((status = m_keydistrib->getKeyToBSB(&m_key1))!= SUCCESS){
        // pl_log_e(TAG," unprotectBufferFromBSB key not retrieved.\n");
        return status;
    }

    return unprotectBufferB(m_key1, buffer, offset, pLen);
}

uint8_t Crypto::macBufferForNodeB(node_id_t nodeID, uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;
    // pl_log_i(TAG,"  macBufferForNodeB called.\n");
    //return status;
    if((status = m_keydistrib->getKeyToNodeB(nodeID, &m_key1)) == SUCCESS){
    //return status;
        status = macBuffer(m_key1, buffer, offset, pLen, buffer + offset + *pLen);
        *pLen = *pLen + MAC_LENGTH;
    } else {        
        // pl_log_e(TAG," macBufferForNodeB failed, key to nodeID %X not found.\n", nodeID); 
        //return SUCCESS;
    }
    
    return status;
}

uint8_t Crypto::macBufferForBSB(uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;

    // pl_log_i(TAG,"  macBufferForBSB called.\n"); 

    if((status =  m_keydistrib->getKeyToBSB(&m_key1)) == SUCCESS){	
        status = macBuffer(m_key1, buffer, offset, pLen, buffer + offset + *pLen);
        *pLen = *pLen + MAC_LENGTH;
    } else {
        // pl_log_e(TAG,"  macBufferForNodeB failed, key to BS not found.\n"); 
    }
    
    return status;       
}

uint8_t Crypto::verifyMacFromNodeB(node_id_t nodeID, uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;        

    // pl_log_i(TAG," verifyMacFromNodeB called.\n"); 
            
    if((status = m_keydistrib->getKeyToNodeB(nodeID, &m_key1)) != SUCCESS){
        // pl_log_e(TAG,"  macBufferForNodeB failed, key to node not found.\n"); 
    }
    
    return verifyMac(m_key1, buffer,  offset, pLen);
}

uint8_t Crypto::verifyMacFromBSB(uint8_t* buffer, uint8_t offset, uint8_t* pLen)
{
    uint8_t status = SUCCESS;        

    // pl_log_i(TAG," verifyMacFromBSB called.\n"); 
            
    if((status = m_keydistrib->getKeyToBSB(&m_key1)) != SUCCESS){
        // pl_log_e(TAG,"  macBufferForBSB failed, key to BS not found.\n");
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
    // uint8_t tempHash[HASH_LENGTH];
    uint8_t status;
    //uint8_t i;

    // pl_log_i(TAG," hashDataShortB called.\n"); 
    if(hash == NULL){
        // pl_log_e(TAG," hashDataShortB NULL hash.\n");
        return FAIL;	    
    }
    
    // if((status = hashDataB(buffer, offset, len, tempHash)) != SUCCESS){
    if((status = m_hash->hashDataB(buffer, offset, len, reinterpret_cast<uint8_t*>(hash))) != SUCCESS){
        // pl_log_e(TAG," hashDataShortB calculation failed.\n"); 
        return status;
    }

    // memcpy(hash, tempHash, sizeof(uint32_t));
    
    return SUCCESS;
}

uint8_t Crypto::verifyHashDataB(uint8_t* buffer, uint8_t offset, uint8_t pLen, uint8_t* hash)
{
    uint8_t status = SUCCESS;
    uint8_t tempHash[BLOCK_SIZE];

    // pl_log_i(TAG," verifyHashDataB called.\n"); 
    if((status = m_hash->hashDataB(buffer, offset, pLen, tempHash)) != SUCCESS){
        // pl_log_e(TAG," verifyHashDataB failed to calculate hash.\n"); 
    }
    if(memcmp(tempHash, hash, BLOCK_SIZE) != 0){
        // pl_log_e(TAG," verifyHashDataB hash not verified.\n"); 
        return EWRONGHASH;
    }
    return status;
}

uint8_t Crypto::verifyHashDataShortB(uint8_t* buffer, uint8_t offset, uint8_t pLen, uint32_t hash)
{
    uint8_t status = SUCCESS;
    uint32_t tempHash = 0;

    // pl_log_i(TAG," verifyHashDataB called.\n");
    if((status = hashDataShortB(buffer, offset, pLen, &tempHash)) != SUCCESS){
        // pl_log_e(TAG," verifyHashDataB failed to calculate hash.\n");
        return FAIL;
    }
    
    if(tempHash != hash){
        // pl_log_e(TAG," verifyHashDataB hash not verified.\n"); 
        return EWRONGHASH;
    }

    return status;
}

// uint8_t Crypto::verifySignature(uint8_t* buffer, uint8_t offset, PRIVACY_LEVEL level, uint16_t counter, Signature_t* signature)
// {

// }

// uint8_t initCryptoIIB(); // TODO!






//
//	CryptoRaw interface
//	

uint8_t Crypto::encryptBufferB(PL_key_t* key, uint8_t* buffer, uint8_t offset, uint8_t len)
{
    uint8_t i;
    uint8_t j;
    uint8_t plainCounter[BLOCK_SIZE];			
    uint8_t encCounter[BLOCK_SIZE];

    // pl_log_d(TAG," encryptBufferB(buffer = '0x%x', 1 = '0x%x', 2 = '0x%x'.\n", buffer[0],buffer[1],buffer[2]);

// if (verifyArgumentsShort("encryptBufferB", key, buffer, offset, len) == FAIL) {
//     return FAIL;
//     }
        
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
            // pl_log_i(TAG,"  encryptBufferB counter overflow, generate new key requiered.\n"); 
            
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
    
    // pl_printf("CryptoP:  verifyMac called.\n"); 
    
// if (verifyArgumentsShort("verifyMac", key, buffer, offset, *pLen) == FAIL) {
//     return FAIL;
//     }

    if (*pLen < MAC_LENGTH){
        // pl_log_e(TAG,"verifyMac input len smaller than mac length. %u\n", *pLen);
        return FAIL;
    }

    macLength = macLength - MAC_LENGTH;
    status = macBuffer(key, buffer, offset, &macLength, mac); //calculate new mac	

    if((memcmp(mac, buffer + offset + *pLen - MAC_LENGTH, MAC_LENGTH)) != 0){ //compare new with received
        return EWRONGMAC;            
        //pl_log_e(TAG,"  verifyMacFromNodeB message MAC does not match.\n"); 
        // return status;
    }
    return status;
}


uint8_t Crypto::deriveKeyB(PL_key_t* masterKey, uint8_t* derivationData, uint8_t offset, uint8_t len, PL_key_t* derivedKey)
{
    // pl_log_d(TAG, " deriveKeyB called.\n"); 

    if(masterKey == NULL){
#ifndef SKIP_SELECTED_CRYPTO_RAW_MESSAGES
    // pl_log_e(TAG," deriveKeyB NULL masterKey.\n");
#endif
    return FAIL;	    
    }
    if(offset > MAX_OFFSET){
#ifndef SKIP_SELECTED_CRYPTO_RAW_MESSAGES
    // pl_log_e(TAG," deriveKeyB offset is larger than max.\n");
#endif
    return FAIL;	    
    }        
    if(len != BLOCK_SIZE){
#ifndef SKIP_SELECTED_CRYPTO_RAW_MESSAGES
    // pl_log_e(TAG," deriveKeyB len != BLOCK_SIZE.\n");
#endif
    return FAIL;	    
    }       
    if(derivationData == NULL){
#ifndef SKIP_SELECTED_CRYPTO_RAW_MESSAGES
    // pl_log_e(TAG," deriveKeyB NULL derivationData.\n");
#endif
    return FAIL;	    
    }        
    if(derivedKey == NULL){
#ifndef SKIP_SELECTED_CRYPTO_RAW_MESSAGES
    // pl_log_e(TAG," deriveKeyB NULL derivedKey.\n");
#endif
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

// if (verifyArgumentsShort("protectBufferB", key, buffer, offset, *pLen) == FAIL) {
//     return FAIL;
//     }
  
    //offset is used for encryption shift, to mac, but not encrypt SPheader
    if((status = macBuffer(key, buffer, 0, pLen, buffer + *pLen)) != SUCCESS){
        // pl_printf("CryptoP:  protectBufferForBSB mac failed.\n");
        return status;
    }
    if((status = encryptBufferB(key, buffer, offset, *pLen)) != SUCCESS){
        // pl_printf("CryptoP:  protectBufferForBSB encrypt failed.\n");
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
    // uint8_t buffer_copy[*pLen];
    // memcpy(buffer_copy, buffer, *pLen);
    
    Serial.print("C");
    Serial.println(*key->counter);
    // pl_log_d(TAG, " unprotectBufferB called.\n");
    //offset is used for encryption shift, to verify SPheader, but not to encrypt it

    // if((status = decryptBufferB(key, buffer, offset, *pLen) != SUCCESS){
    if((status = decryptBufferB(key, buffer, offset, *pLen - m_mac->macSize())) != SUCCESS){
        // pl_log_e(TAG, "  unprotectBufferB encrypt failed.\n");
        return status;		
    }

// TODO! REMOVE
#ifndef __linux__
    printBuffer(buffer, *pLen);
#endif //  __linux__

    if((status = verifyMac(key, buffer, 0, pLen)) != SUCCESS){            
        // pl_log_e(TAG, "  unprotectBufferB mac verification failed, trying to sychronize counter.\n"); 
        // memcpy(buffer, buffer_copy, *pLen);
        // printBuffer(buffer, *pLen);
        for (i = 1; i <= COUNTER_SYNCHRONIZATION_WINDOW; i++){    
            *(key->counter) = counter - i;
            decryptBufferB(key, buffer, offset, *pLen- m_mac->macSize());
            if((status = verifyMac(key, buffer, 0, pLen)) == SUCCESS){
                // pl_log_i(TAG, " counter synchronization succesfull.\n");
                return status;
            }
    
            *(key->counter) = counter + i;
            decryptBufferB(key, buffer, offset, *pLen - m_mac->macSize());
            if((status = verifyMac(key, buffer, 0, pLen)) == SUCCESS){
                // pl_log_i(TAG, " counter synchronization succesfull.\n");
                return status;
            }
        }
        // pl_log_e(TAG, " counter could not be sychronized, decrypt failed.\n");
        *(key->counter) = counter;
        return status;
    }
    return status;
}
