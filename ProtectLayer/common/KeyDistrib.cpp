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

// TODO create single header with EEPROM-related stuff shared by PL and Configurator
#define CONFIG_START_ADDRESS    (uint8_t*)0x40
#define KEY_STRUCT_SIZE         sizeof(PL_key_t)

KeyDistrib::KeyDistrib()
{
    // zero out the current key and all counters
    memset((void*) &m_key, 0, sizeof(PL_key_t));
    memset((void*) m_counters, 0, (MAX_NODE_NUM + 1) * sizeof(uint32_t));
}

uint8_t KeyDistrib::getKeyToNodeB(uint8_t nodeID, PL_key_t** pNodeKey)
{
    // read the key
    eeprom_read_block(m_key.keyValue, CONFIG_START_ADDRESS + (nodeID * AES_KEY_SIZE), AES_KEY_SIZE);// TODO! REMOVE
    // set the counter
    m_key.counter = m_counters + nodeID;
    // set pointer
    *pNodeKey = &m_key;

    return SUCCESS;
}

uint8_t KeyDistrib::getKeyToBSB(PL_key_t** pBSKey)
{
    eeprom_read_block(m_key.keyValue, CONFIG_START_ADDRESS, AES_KEY_SIZE);
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
    // overwrite the key
    for(int i=0;i<AES_KEY_SIZE;i++){
        eeprom_write_byte(CONFIG_START_ADDRESS + (nodeID * AES_KEY_SIZE) + i, 0);
    }

    return SUCCESS;
}

#endif // __linux__