/**
 * @brief Implementation of uTESLA for Linux base station using slave device connected through a serial port
 * 
 * @file uTESLAMaster.cpp
 * @author Martin Sarkany
 * @date 05/2018
 */

#ifdef __linux__

#include "uTESLAMaster.h"

#include <iostream>
#include <iomanip>
#include <algorithm>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#include "AES_crypto.h"
#include "ProtectLayerGlobals.h"


uTeslaMaster::uTeslaMaster(const int32_t device_fd, const uint8_t *initial_key, const uint32_t rounds_num, Hash *hash, MAC *mac): m_hash(hash), m_mac(mac)
{
    // set attributes
    m_dev_fd = device_fd;
    m_rounds_num = rounds_num;
    m_current_key_index = m_rounds_num - 1;
    m_hash_size = hash->hashSize();
    m_mac_size = mac->macSize();
    m_mac_key_size = mac->keySize();
    m_hash_chain.resize(rounds_num + 2);

    m_hash_chain[0] = new uint8_t[m_hash_size];
    memcpy(m_hash_chain[0], initial_key, m_hash_size);

    // compute the hash chain
// #ifdef DEBUG
//    std::cout << "Hash chain:" << std::endl;
// #endif
    for(uint32_t i=1;i<rounds_num + 1;i++){
        m_hash_chain[i] = new uint8_t[m_hash_size];
        if(!m_hash->hash(m_hash_chain[i-1], m_hash_size, m_hash_chain[i], m_hash_size)){
            // std::cerr << "Failed to initialize hash chain" << std::endl;
            uTeslaMasterException ex("Failed to initialize hash chain");
            throw ex;
        }
// #ifdef DEBUG
//        printBufferHex(m_hash_chain[i], m_hash_size);// << std::endl;
// #endif
    }

}

uTeslaMaster::~uTeslaMaster()
{
    // destroy the hash chain
    for(uint32_t i=0;i<m_rounds_num + 1;i++){
        delete[] m_hash_chain[i];
    }
}

void uTeslaMaster::printLastElementHex()
{
    printBufferHex(m_hash_chain[m_rounds_num], m_hash_size);
}

uint8_t uTeslaMaster::broadcastKey()
{
    // buffer size is hash size + length twice + header size
    int32_t buffer_size = m_hash_size + 2 + SPHEADER_SIZE;
    uint8_t buffer[MAX_MSG_SIZE];

    // set length for serial communication
    buffer[0] = buffer_size - 2;
    buffer[1] = buffer_size - 2;

    // set header
    SPHeader_t *spheader = reinterpret_cast<SPHeader_t*>(buffer + 2);
    spheader->msgType = MSG_UTESLA_KEY;
    spheader->sender = BS_NODE_ID;
    spheader->receiver = 0;

    // compy key to buffer
    memcpy(buffer + 2 + SPHEADER_SIZE, m_hash_chain[m_current_key_index], m_hash_size);

    // send to serial port
    if(write(m_dev_fd, buffer, buffer_size) < buffer_size){
        return FAIL;
    }

    return SUCCESS;
}

uint8_t uTeslaMaster::newRound()
{
    // return FAIL if there are no keys left
    if(m_current_key_index < 0){
        std::cerr << "Key index"  << std::endl; // TODO REMOVE!
        return FAIL;
    }

    // broadcast the key
    if(broadcastKey() != SUCCESS){
        std::cerr << "broadcast"  << std::endl; // TODO REMOVE!
        return FAIL;
    }

    m_current_key_index--;

    return SUCCESS;
}

uint8_t uTeslaMaster::broadcastMessage(const uint8_t* data, const uint16_t data_len)
{
    if(!data){
        printDebug("NULL message to broadcast", true);
        return FAIL;
    }

    if(m_current_key_index < 0){
        printDebug("Out of uTESLA rounds", true);
        return FAIL;
    }

    if(data_len > MAX_MSG_SIZE - SPHEADER_SIZE - m_mac_size){
        printDebug("Data too long", true);
        return FAIL;
    }

    // set packet size to size of the message, MAC, HEADER and 2 bytes for serial communication
    int packet_size = data_len + m_mac_size + 2 + SPHEADER_SIZE;

    uint8_t buffer[MAX_MSG_SIZE];
    buffer[0] = packet_size - 2;
    buffer[1] = packet_size - 2;
    
    // set the header
    SPHeader_t *spheader = reinterpret_cast<SPHeader_t*>(buffer + 2);
    spheader->msgType = MSG_UTESLA;
    spheader->sender = BS_NODE_ID;
    spheader->receiver = 0;

    // copy yo buffer
    memcpy(buffer + 2 + SPHEADER_SIZE, data, data_len);

    // compute MAC
    if(!m_mac->computeMAC(m_hash_chain[m_current_key_index], m_mac_key_size, buffer + 2, data_len + SPHEADER_SIZE, buffer + 2 + SPHEADER_SIZE + data_len, m_mac_size)){
        printDebug("Failed to compute MAC", true);
        return FAIL;
    }

    // send to serial port
    if(write(m_dev_fd, buffer, packet_size) < packet_size){
        printDebug("Failed to broadcast message", true);
        return FAIL;
    }

    return SUCCESS;
}

#endif // __linux__
