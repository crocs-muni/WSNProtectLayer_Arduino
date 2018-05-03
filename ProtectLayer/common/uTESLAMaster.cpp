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


void printBufferHex(const uint8_t *buffer, const uint32_t len)
{
    for(uint32_t i=0;i<len;i++){
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}

// TODO! move to separate file
void uTeslaMaster::openSerialPort(std::string &serial_port)
{
    // open file descriptor
    m_dev_fd = open(serial_port.c_str() , O_RDWR | O_NOCTTY);
    if(!m_dev_fd){
        uTeslaMasterException ex("Failed to open serial port");
        throw ex;
    }

    // set parameters
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if(tcgetattr(m_dev_fd, &tty) != 0)    {
        uTeslaMasterException ex("Failed to open serial port");
        throw ex;
    }

    if(cfgetispeed(&tty) != B115200 && cfsetispeed(&tty, B115200)){
        uTeslaMasterException ex("Failed to set input baud rate");
        throw ex;
    }

    if(cfgetospeed(&tty) != B115200 && cfsetospeed(&tty, B115200)){
        uTeslaMasterException ex("Failed to set output baud rate");
        throw ex;
    }

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_lflag = 0;
    tty.c_cc[VMIN]  = 1;
    tty.c_cc[VTIME] = 5;
    if (tcsetattr(m_dev_fd, TCSANOW, &tty) != 0)    {
        uTeslaMasterException ex("Failed to set flags for serial port");
        throw ex;
    }
}

uTeslaMaster::uTeslaMaster(std::string &serial_port, const uint8_t *initial_key, const uint32_t rounds_num, Hash *hash, MAC *mac): m_hash(hash), m_mac(mac)
{
    m_rounds_num = rounds_num;
    m_current_key_index = m_rounds_num - 1;
    m_hash_size = hash->hashSize();
    m_mac_size = mac->macSize();
    m_mac_key_size = mac->keySize();
    m_hash_chain.resize(rounds_num + 1);

    m_hash_chain[0] = new uint8_t[m_hash_size];
    memcpy(m_hash_chain[0], initial_key, m_hash_size);

// #ifdef DEBUG
//     std::cout << "Hash chain:" << std::endl;
// #endif
    
    for(uint32_t i=1;i<rounds_num + 1;i++){
        m_hash_chain[i] = new uint8_t[m_hash_size];
        if(!m_hash->hash(m_hash_chain[i-1], m_hash_size, m_hash_chain[i], m_hash_size)){
            // std::cerr << "Failed to initialize hash chain" << std::endl;
            uTeslaMasterException ex("Failed to initialize hash chain");
            throw ex;
        }
// #ifdef DEBUG
//         printBufferHex(m_hash_chain[i], m_hash_size);// << std::endl;
// #endif
    }

    openSerialPort(serial_port);
}

uTeslaMaster::uTeslaMaster(const int32_t device_fd, const uint8_t *initial_key, const uint32_t rounds_num, Hash *hash, MAC *mac): m_hash(hash), m_mac(mac)
{
    m_dev_fd = device_fd;
    m_rounds_num = rounds_num;
    m_current_key_index = m_rounds_num - 1;
    m_hash_size = hash->hashSize();
    m_mac_size = mac->macSize();
    m_mac_key_size = mac->keySize();
    m_hash_chain.resize(rounds_num + 2);

    m_hash_chain[0] = new uint8_t[m_hash_size];
    memcpy(m_hash_chain[0], initial_key, m_hash_size);

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
    // send it to arduino
    // maybe size twice first
    int32_t buffer_size = m_hash_size + 2 + SPHEADER_SIZE;
    // uint8_t buffer[buffer_size];
    uint8_t buffer[MAX_MSG_SIZE];

    buffer[0] = buffer_size - 2;
    buffer[1] = buffer_size - 2;
    // buffer[2] = MSG_TYPE_KEY;

    SPHeader_t *spheader = reinterpret_cast<SPHeader_t*>(buffer + 2);
    spheader->msgType = MSG_UTESLA_KEY;
    spheader->sender = BS_NODE_ID;
    spheader->receiver = 0;

    memcpy(buffer + 2 + SPHEADER_SIZE, m_hash_chain[m_current_key_index], m_hash_size);

    if(write(m_dev_fd, buffer, buffer_size) < buffer_size){
        return FAIL;
    }

    return SUCCESS;
}

uint8_t uTeslaMaster::newRound()
{
    if(m_current_key_index < 0){
        std::cerr << "Key index"  << std::endl; // TODO REMOVE!
        return FAIL;
    }

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

    // int buffer_size = data_len + m_mac_size + 2;
    int packet_size = data_len + m_mac_size + 2 + SPHEADER_SIZE;

    uint8_t buffer[MAX_MSG_SIZE];
    buffer[0] = packet_size - 2;
    buffer[1] = packet_size - 2;
    
    SPHeader_t *spheader = reinterpret_cast<SPHeader_t*>(buffer + 2);
    spheader->msgType = MSG_UTESLA;
    spheader->sender = BS_NODE_ID;
    spheader->receiver = 0;

    memcpy(buffer + 2 + SPHEADER_SIZE, data, data_len);

    if(!m_mac->computeMAC(m_hash_chain[m_current_key_index], m_mac_key_size, buffer + 2, data_len + SPHEADER_SIZE, buffer + 2 + SPHEADER_SIZE + data_len, m_mac_size)){
        printDebug("Failed to compute MAC", true);
        // std::cerr << "Failed to compute MAC" << std::endl;
        return FAIL;
    }

    if(write(m_dev_fd, buffer, packet_size) < packet_size){
        printDebug("Failed to broadcast message", true);
        // std::cerr << "Failed to broadcast message" << std::endl;
        return FAIL;
    }

    return SUCCESS;
}

#endif // __linux__
