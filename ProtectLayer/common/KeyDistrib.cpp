/**
 * @brief Implementation of key handling for both BS and ordinary nodes
 * 
 * @file    KeyDistrib.cpp
 * @author  Martin Sarkany
 * @date    05/2018
 */

#include "KeyDistrib.h"

#include <string.h>


#ifdef  __linux__
#include "configurator.h"

KeyDistrib::KeyDistrib(std::string &filename)
{
    // init configurator with keys
    Configurator configurator(filename, 0);

    // set attributes
    m_nodes = configurator.getNodes();
    m_nodes_num = m_nodes.size();
    m_key_size = configurator.getKeySize();
    memset(m_counters, 0, (MAX_NODE_NUM + 1) * sizeof(uint32_t));
}

uint8_t KeyDistrib::getKeyToNodeB(uint8_t nodeID, PL_key_t** pNodeKey)
{
    // find the wanted node, set its counter and pointer to it
    for(int i=0;i<m_nodes_num;i++){
        if(m_nodes[i].ID == nodeID){
            memcpy(m_key.keyValue, m_nodes[i].BS_key.data(), m_key_size);
            m_key.counter = m_counters + i;
            *pNodeKey = &m_key;
            return SUCCESS;
        }
    }

    return FAIL;
}

uint8_t KeyDistrib::getKeyToBSB(PL_key_t** pNodeKey)
{
    return FAIL;
}

uint8_t KeyDistrib::getHashKeyB(PL_key_t** pHashKey)
{
    // always set to 0 in original WSNProtectLayer
    *m_counters = 0;
    m_key.counter = m_counters;
    memset(m_key.keyValue, 0, AES_KEY_SIZE);
    *pHashKey = &m_key;

    return SUCCESS;
}

#else  // __linux__

#include <avr/eeprom.h>
#include "common.h"


KeyDistrib::KeyDistrib(uint32_t *neighbors): m_neighbors(neighbors)
{
    // zero out the current key and all counters
    memset((void*) &m_key, 0, sizeof(PL_key_t));
    memset((void*) m_counters, 0, (MAX_NODE_NUM + 1) * sizeof(uint32_t));

    eeprom_read_block(&m_nodes_list, NODES_LIST_ADDRESS, sizeof(uint32_t));
}

uint8_t KeyDistrib::getKeyToNodeB(uint8_t nodeID, PL_key_t** pNodeKey)
{
    if(nodeID < 2){
        return FAIL;
    }
    // check if key is configured
    if(!(bitIsSet(m_nodes_list, nodeID))){
        return FAIL;
    }

    if(bitIsSet(*m_neighbors, nodeID)){
        return getDerivedKeyToNodeB(nodeID, pNodeKey);
    }

    // read the key
    eeprom_read_block(m_key.keyValue, KEYS_START_ADDRESS + ((nodeID - 1) * AES_KEY_SIZE), AES_KEY_SIZE);
    // set the counter
    m_key.counter = m_counters + nodeID;
    // set pointer
    *pNodeKey = &m_key;

    return SUCCESS;
}

uint8_t KeyDistrib::getDerivedKeyToNodeB(uint8_t nodeID, PL_key_t** pNodeKey)
{
    if(nodeID < 2){
        return FAIL;
    }

    if(!(bitIsSet(*m_neighbors, nodeID))){
        return FAIL;
    }

    // read the key
    eeprom_read_block(m_key.keyValue, DRVD_KEYS_START_ADDRESS + ((nodeID - 1) * AES_KEY_SIZE), AES_KEY_SIZE);
    // set the counter
    m_key.counter = m_counters + nodeID;
    // set pointer
    *pNodeKey = &m_key;

    return SUCCESS;
}

uint8_t KeyDistrib::getKeyToBSB(PL_key_t** pBSKey)
{
    eeprom_read_block(m_key.keyValue, KEYS_START_ADDRESS, AES_KEY_SIZE);
    m_key.counter = m_counters + 1;
    *pBSKey = &m_key;

    return SUCCESS;
}

uint8_t KeyDistrib::getHashKeyB(PL_key_t** pHashKey)
{
    // always set to 0 in original WSNProtectLayer
    *m_counters = 0;
    m_key.counter = m_counters;
    memset(m_key.keyValue, 0, AES_KEY_SIZE);
    *pHashKey = &m_key;

    return SUCCESS;
}

uint8_t KeyDistrib::deleteKey(uint8_t nodeID)
{
    if(nodeID < 2){
        return FAIL;
    }

    if(!(bitIsSet(m_nodes_list, nodeID))){
        return FAIL;
    }

    // overwrite the key
    for(int i=0;i<AES_KEY_SIZE;i++){
        eeprom_write_byte(KEYS_START_ADDRESS + ((nodeID - 1) * AES_KEY_SIZE) + i, 0);
    }

    if(bitIsSet(*m_neighbors, nodeID)){
        for(int i=0;i<AES_KEY_SIZE;i++){
            eeprom_write_byte(DRVD_KEYS_START_ADDRESS + ((nodeID - 1) * AES_KEY_SIZE) + i, 0);
        }
    }

    return SUCCESS;
}

uint8_t KeyDistrib::deriveKeyToNode(uint8_t nodeID, uint8_t *random_input, uint8_t random_input_size, MAC *mac)
{
    uint8_t mac_buff[AES_MAC_SIZE];
    uint8_t original_key[AES_KEY_SIZE];

#if AES_MAC_SIZE != AES_KEY_SIZE
#error AES_MAC_SIZE is not equal to AES_KEY_SIZE
#endif

    eeprom_read_block(original_key, KEYS_START_ADDRESS + ((nodeID - 1) * AES_KEY_SIZE), AES_KEY_SIZE);

    if(random_input_size != 16){
        return FAIL;
    }

    // if(hash->hash(random_input, random_input_size, hash_buff, AES_HASH_SIZE) != true) // TODO SUCCES instead of true in AEShash class
    if(mac->computeMAC(original_key, AES_KEY_SIZE, random_input, random_input_size, original_key, AES_MAC_SIZE) != true) // TODO SUCCES instead of true in AEShash class

    // for(int i=0;i<AES_KEY_SIZE;i++){
    //     original_key[i] ^= mac_buff[i];
    // }

    eeprom_update_block(original_key, DRVD_KEYS_START_ADDRESS + ((nodeID - 1) * AES_KEY_SIZE), AES_KEY_SIZE);

    return SUCCESS;
}

uint32_t KeyDistrib::getNodesList()
{
    return m_nodes_list;
}

#endif // __linux__