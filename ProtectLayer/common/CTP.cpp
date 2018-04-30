#include "CTP.h"


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

    // read(m_slave_fd, buffer, MAX_MSG_SIZE);

    memset(buffer, 0, MAX_MSG_SIZE);
    SPHeader_t header = { MSG_CTP, 0, 0 };

    buffer[0] = sizeof(SPHeader_t) + 1;
    buffer[1] = buffer[0];
    memcpy(buffer + 2, &header, sizeof(SPHeader_t));
    buffer[sizeof(SPHeader_t)] = 0;

    for(int i=0;i<CTP_REBROADCASTS_NUM;i++){
        int len;
        if((len = write(m_slave_fd, buffer, sizeof(SPHeader_t) + 3)) < sizeof(SPHeader_t) + 3){
            return FAIL;
        }

        if((len = read(m_slave_fd, buffer, MAX_MSG_SIZE)) < 1){
            return FAIL;
        }

        if(buffer[0] != ERR_OK){
            return FAIL;
        }
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
m_parent_id(0), m_distance(50), m_req_ack(DEFAULT_REQ_ACK)
{ 

}

void CTP::setNodeID(uint8_t node_id)
{
    m_node_id = node_id;
}

// extracts distance from packet
void CTP::update(uint8_t *message)
{
    if(((SPHeader_t*)(message))->msgType != MSG_CTP){
        return;
    }

    if(message[sizeof(SPHeader_t)] + 1 >= m_distance){
        return;
    }

    m_distance = message[sizeof(SPHeader_t)] + 1;
    m_parent_id = ((SPHeader_t*)(message))->sender;
}

// receives distance messages until end and sets variables accordingly
void CTP::handleDistanceMessages(uint32_t end)
{
    uint8_t rcvd_msg[sizeof(SPHeader_t) + 1];
    uint8_t rcvd_msg_len;
    // uint8_t rcvd_msg_hdr;

    while(waitReceive(end)){
        replyAck();
        if(rf12_len == sizeof(SPHeader_t) + 1){
            rcvd_msg_len = rf12_len;
            // rcvd_msg_hdr = rf12_hdr;
            memcpy( rcvd_msg, rf12_data, rcvd_msg_len);    
        }
        rf12_recvDone();

        update(rcvd_msg);
    }
}


// broadcasts it's distance from BS
void CTP::broadcastDistance(){
    uint8_t buffer[sizeof(SPHeader_t) + 1];
    SPHeader_t *header = reinterpret_cast<SPHeader_t*>(buffer);
    header->msgType = MSG_CTP;
    header->sender = m_node_id;
    header->receiver = 0;
    buffer[sizeof(SPHeader_t)] = m_distance;

    uint8_t rf12_header = createHeader(0, MODE_SRC, m_req_ack);
    rf12_sendNow(rf12_header, buffer, sizeof(SPHeader_t) + 1);
}


// routing table establishment phase main function for non-BS nodes
uint8_t CTP::startCTP(uint32_t duration)
{
    uint32_t start = millis();
    uint32_t ms;

    while((ms = millis()) <= start + duration){
        // handle distance messages for half a second
        handleDistanceMessages(ms + 300);

        // broadcast own distance
        broadcastDistance();
        rf12_sendWait(0);
        
        // if there is time left (should be), continue receiving distance messages
        handleDistanceMessages(ms + 300);
    }

    if(m_distance >= 50 || m_parent_id == 0){
        return FAIL;
    }

    return SUCCESS;
}

void CTP::send(uint8_t *buffer, uint8_t length)
{
    uint8_t header = createHeader(m_parent_id, MODE_DST, m_req_ack);
    rf12_sendNow(header, buffer, length);
}

uint8_t CTP::getParentID()
{
    return m_parent_id;
}

#endif