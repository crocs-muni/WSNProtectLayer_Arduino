/**
 * @brief Stripped-down implementation of a CTP protocol for Linux BS and JeeLink devices
 * 
 * @file    CTP.cpp
 * @author  Martin Sarkany
 * @date    05/2018
 */

#include "CTP.h"

#include "common.h"

#ifdef __linux__    // BS host
#include <chrono>
#include <thread>

#include <cstring>
#include <unistd.h>


void CTP::setSlaveFD(int slave_fd)
{
    m_slave_fd = slave_fd;
}

uint8_t CTP::startCTP(uint32_t duration)
{
    uint64_t start = millis();
    uint8_t buffer[MAX_MSG_SIZE];
    uint8_t recv_buffer[MAX_MSG_SIZE];

    memset(buffer, 0, MAX_MSG_SIZE);

    // set size of message for serial communication
    buffer[0] = SPHEADER_SIZE + 1;
    buffer[1] = buffer[0];
    
    // set header pointer
    SPHeader_t *header = reinterpret_cast<SPHeader_t*>(buffer + 2);
    header->msgType = MSG_CTP;
    header->sender = BS_NODE_ID;
    header->receiver = 0;

    // set distance to 0
    buffer[SPHEADER_SIZE + 2] = 0;

    // rebroadcast several times
    for(int i=0;i<CTP_REBROADCASTS_NUM;i++){
        int len;
        
        if((len = write(m_slave_fd, buffer, sizeof(SPHeader_t) + 3)) < sizeof(SPHeader_t) + 3){
            return FAIL;
        }

        if((len = read(m_slave_fd, recv_buffer, MAX_MSG_SIZE)) < 1){
            return FAIL;
        }
        
        if(recv_buffer[0] != ERR_OK){
           return FAIL;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(CTP_REBROADCASTS_DELAY));
    }

    // sleep until CTP formation ends
    uint64_t end = millis();
    if(end - start < duration){
        std::this_thread::sleep_for(std::chrono::milliseconds(duration - (end - start)));
    }

    return SUCCESS;
}

#else   // node
#include "RF12.h"

CTP::CTP(): 
m_node_id(0), m_parent_id(0), m_distance(INVALID_DISTANCE), m_req_ack(DEFAULT_REQ_ACK)
{ 

}

void CTP::setNodeID(uint8_t node_id)
{
    m_node_id = node_id;
}

// extracts distance from packet
void CTP::update(uint8_t *message)
{
    Serial.println(message[3]);

    // check header
    if(((SPHeader_t*)(message))->msgType != MSG_CTP){
        return;
    }

    // ignore longer distances
    if(message[sizeof(SPHeader_t)] + 1 >= m_distance){
        return;
    }

    // set attributes
    m_distance = message[sizeof(SPHeader_t)] + 1;
    m_parent_id = ((SPHeader_t*)(message))->sender;
}

// receives distance messages until end and sets variables accordingly
void CTP::handleDistanceMessages(uint32_t end)
{
    uint8_t rcvd_msg[sizeof(SPHeader_t) + 1];
    uint8_t rcvd_msg_len;

    while(waitReceive(end)){
        replyAck();
        if(rf12_len == sizeof(SPHeader_t) + 1){
            rcvd_msg_len = rf12_len;
            memcpy( rcvd_msg, rf12_data, rcvd_msg_len);    
        }
        rf12_recvDone();

        update(rcvd_msg);
    }
}

// broadcasts it's distance from BS
void CTP::broadcastDistance(){
    if(m_distance == INVALID_DISTANCE){   // TODO use define
        return;
    }

    uint8_t buffer[sizeof(SPHeader_t) + 1];

    // set header
    SPHeader_t *header = reinterpret_cast<SPHeader_t*>(buffer);
    header->msgType = MSG_CTP;
    header->sender = m_node_id;
    header->receiver = 0;

    // set distance
    buffer[sizeof(SPHeader_t)] = m_distance;

    // send
    uint8_t rf12_header = createHeader(0, MODE_SRC, m_req_ack);
    rf12_sendNow(rf12_header, buffer, sizeof(SPHeader_t) + 1);
}

// routing table establishment phase main function for non-BS nodes
uint8_t CTP::startCTP(uint32_t duration)
{
    uint32_t start = millis();
    uint32_t ms;

    while((ms = millis()) < start + duration){
        // handle distance messages for half a second
        handleDistanceMessages(ms + 500);

        // broadcast own distance
        broadcastDistance();
        // TODO ?

        // if there is time left (should be), continue receiving distance messages
        handleDistanceMessages(ms + 500);
    }

    if(m_distance >= INVALID_DISTANCE || m_parent_id == 0){
        Serial.println(m_distance);
        Serial.println(m_parent_id);
        return FAIL;
    }

    return SUCCESS;
}

// void CTP::send(uint8_t *buffer, uint8_t length)
// {
//     uint8_t header = createHeader(m_parent_id, MODE_DST, m_req_ack);
//     rf12_sendNow(header, buffer, length);
// }

uint8_t CTP::getParentID()
{
    return m_parent_id;
}

#endif
