#include "KeyDistrib.h"

#include <string.h>


#ifndef __linux__
#include <avr/eeprom.h>
#include "common.h"

// TODO create single header with EEPROM-related stuff shared by PL and Configurator
#define CONFIG_START_ADDRESS    (uint8_t*)0x40
#define KEY_STRUCT_SIZE         sizeof(PL_key_t)

KeyDistrib::KeyDistrib()
{
    memset((void*) &m_key, 0, sizeof(PL_key_t));
    memset((void*) m_counters, 0, MAX_NODE_NUM * sizeof(uint32_t));
}
//
// uint8_t KeyDistrib::discoverKeys()
// {
//     // TODO!
//     return FAIL;
// }


uint8_t KeyDistrib::getKeyToNodeB(uint8_t nodeID, PL_key_t** pNodeKey)
{
    // m_key.counter = 0;
    eeprom_read_block(m_key.keyValue, CONFIG_START_ADDRESS + (nodeID * KEY_SIZE), KEY_SIZE);// TODO! REMOVE
    m_key.counter = m_counters + nodeID;
    *pNodeKey = &m_key;

    return SUCCESS;
}

uint8_t KeyDistrib::getKeyToBSB(PL_key_t** pBSKey)
{
    // m_key.counter = 0;
    eeprom_read_block(m_key.keyValue, CONFIG_START_ADDRESS, KEY_SIZE);
    m_key.counter = m_counters + 1;
    *pBSKey = &m_key;

    return SUCCESS;
}

uint8_t KeyDistrib::getHashKeyB(PL_key_t** pHashKey)
{
    // set to 0 in original WSNProtectLayer
    *(m_key.counter) = 0;
    memset(m_key.keyValue, 0, KEY_SIZE);
    *pHashKey = &m_key;

    return SUCCESS;
}

uint8_t KeyDistrib::deleteKey(uint8_t nodeID)
{
    for(int i=0;i<KEY_SIZE;i++){
        eeprom_write_byte(CONFIG_START_ADDRESS + (nodeID * KEY_SIZE) + i, 0);
    }

    return SUCCESS;
}

#else // __linux__
#include "configurator.h"

KeyDistrib::KeyDistrib(std::string &filename)
{
    Configurator configurator(filename, 0);
    m_nodes = configurator.getNodes();
    m_nodes_num = m_nodes.size();
    m_key_size = configurator.getKeySize();
}

uint8_t KeyDistrib::getKeyToNodeB(uint8_t nodeID, PL_key_t** pNodeKey)
{
    for(int i=0;i<m_nodes_num;i++){
        if(m_nodes[i].ID == nodeID){
            memcpy(m_key.keyValue, m_nodes[i].BS_key.data(), m_key_size);
            *pNodeKey = &m_key;
            return SUCCESS;
        }
    }

    return FAIL;
}

uint8_t KeyDistrib::getKeyToBSB(PL_key_t** pBSKey)
{
    return FAIL;
}

uint8_t KeyDistrib::getHashKeyB(PL_key_t** pHashKey)
{
    // set to 0 in original WSNProtectLayer
    m_key.counter = 0;
    memset(m_key.keyValue, 0, KEY_SIZE);
    *pHashKey = &m_key;

    return SUCCESS;
}

uint8_t KeyDistrib::deleteKey(uint8_t nodeID)
{
    return FAIL;
}

#endif // __linux__