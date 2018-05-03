#include "ProtectLayer.h"


#ifdef __linux__
#include "configurator.h"

#include <termios.h>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>


int openSerialPort(std::string path);   // TODO move from configurator to separate file with header


ProtectLayer::ProtectLayer(std::string &slave_path, std::string &key_file):
m_hash(&m_aes), m_mac(&m_aes), m_keydistrib(key_file), m_crypto(&m_aes, &m_mac, &m_hash, &m_keydistrib), m_node_id(BS_NODE_ID)
{ 
    m_slave_fd = openSerialPort(slave_path);
    if(m_slave_fd < 0){
        throw std::runtime_error("Failed to open serial port");
    }
// Configurator(std::string &in_filename, const int uTESLA_rounds, const int key_size = 0);
    m_ctp.setSlaveFD(m_slave_fd);
    Configurator configurator(key_file, 0, 0);
    m_utesla = new uTeslaMaster(m_slave_fd, configurator.getuTESLAKey(), configurator.getuTESLARounds(), &m_hash, &m_mac);

    // fire up the device
    uint8_t buffer[MAX_MSG_SIZE];
    read(m_slave_fd, buffer, MAX_MSG_SIZE);
}

ProtectLayer::~ProtectLayer()
{
    delete m_utesla;
    if(m_slave_fd > 0){
        close(m_slave_fd);
    }
}

uint8_t ProtectLayer::broadcastMessage(uint8_t *buffer, uint8_t size)
{
    return m_utesla->broadcastMessage(buffer, size);
}

uint8_t ProtectLayer::broadcastKey()
{
    return m_utesla->newRound();
}

uint8_t ProtectLayer::startCTP()
{
    return m_ctp.startCTP(CTP_DURATION_MS);
}

uint8_t ProtectLayer::sendTo(msg_type_t msg_type, uint8_t receiver, uint8_t *buffer, uint8_t size)
{
    if(size > MAX_MSG_SIZE + SPHEADER_SIZE + m_mac.macSize() || receiver < 2 || !buffer){
        return FAIL;
    }
    
    uint8_t msg_buffer[MAX_MSG_SIZE + 2];
    SPHeader_t *spheader = reinterpret_cast<SPHeader_t*>(msg_buffer + 2);

    if(msg_type != MSG_OTHER && msg_type != MSG_APP){
        return FAIL;
    }

    spheader->msgType = msg_type;
    spheader->sender = BS_NODE_ID;
    spheader->receiver = receiver;

    memcpy(msg_buffer + 2 + SPHEADER_SIZE, buffer, size);

    size += SPHEADER_SIZE;
    if(m_crypto.protectBufferForNodeB(receiver, msg_buffer + 2, SPHEADER_SIZE, &size) != SUCCESS){
        return FAIL;
    }

    msg_buffer[0] = size;
    msg_buffer[1] = size;

    int wr_len = 0;
    if((wr_len = write(m_slave_fd, buffer, size + 2)) != size + 2){
        return FAIL;
    }

    int rd_len = 0;
    if((rd_len = read(m_slave_fd, buffer, MAX_MSG_SIZE)) != 1){
        return FAIL;
    }

    if(buffer[0] != ERR_OK){
        return FAIL;
    }

    return SUCCESS;
}

uint8_t ProtectLayer::receive(uint8_t *buffer, uint8_t buff_size, uint8_t *received_size)
{
    uint8_t rcvd_len = 0;
    uint8_t rcvd_buff[MAX_MSG_SIZE + 10];

    if((rcvd_len = read(m_slave_fd, (void*) rcvd_buff, MAX_MSG_SIZE)) < SPHEADER_SIZE + m_mac.macSize()){
        return FAIL;
    }

    if(rcvd_len > MAX_MSG_SIZE){    // cannot happen
        return ERR_BUFFSIZE;
    }

    SPHeader_t *spheader = reinterpret_cast<SPHeader_t*>(rcvd_buff);
    uint8_t rval;

#ifdef DEBUG
    printBufferHex(rcvd_buff, rcvd_len);// TODO! REMOVE
#endif

    if(spheader->receiver != BS_NODE_ID){
        return FAIL;
    }


    if((rval = m_crypto.unprotectBufferFromNodeB(spheader->sender, rcvd_buff, (uint8_t) SPHEADER_SIZE, &rcvd_len)) != SUCCESS){
        return FAIL;
    }

    *received_size = rcvd_len - m_mac.macSize();

    if(*received_size > buff_size){
        return FAIL;
    }

    memcpy(buffer, rcvd_buff, *received_size);

    return SUCCESS;
}

uint8_t ProtectLayer::getNodeID()
{
    return BS_NODE_ID;
}

#else
#include "RF12.h"
#include <avr/eeprom.h>

#ifdef ENABLE_UTESLA
// initialize also uTESLA
ProtectLayer::ProtectLayer():
m_hash(&m_aes), m_mac(&m_aes), m_crypto(&m_aes, &m_mac, &m_hash, &m_keydistrib), m_neighbors(0), m_utesla((int8_t*) 0x1F4, &m_hash, &m_mac)
#else
// do not initialize uTESLA
ProtectLayer::ProtectLayer():
m_hash(&m_aes), m_mac(&m_aes), m_crypto(&m_aes, &m_mac, &m_hash, &m_keydistrib)
#endif
{
    Serial.begin(BAUD_RATE);

    memset(m_received, 0, 2);
    m_node_id = eeprom_read_byte(0);
    m_ctp.setNodeID(m_node_id);

    rf12_initialize(m_node_id, RADIO_FREQ, RADIO_GROUP);
}


uint8_t ProtectLayer::startCTP()
{
    return m_ctp.startCTP(CTP_DURATION_MS);
}

uint8_t ProtectLayer::sendCTP(msg_type_t msg_type, uint8_t *buffer, uint8_t size)
{
    // TODO not implemented yet
    return sendTo(msg_type, m_ctp.getParentID(), buffer, size);
}

uint8_t ProtectLayer::sendTo(msg_type_t msg_type, uint8_t receiver, uint8_t *buffer, uint8_t size)
{
    if(!buffer || size + SPHEADER_SIZE > MAX_MSG_SIZE){ // TODO size can be bigger when using block cipher
        return FAIL;
    }

    if(receiver < 1 || receiver > 30){
        return FAIL;
    }

    uint8_t pLen = size + SPHEADER_SIZE;
    uint8_t msg_buffer[MAX_MSG_SIZE];   // TODO use some common buffer
    SPHeader_t *header = reinterpret_cast<SPHeader_t*>(msg_buffer);
    memset(msg_buffer, 0, MAX_MSG_SIZE);

    header->msgType = msg_type;
    header->receiver = receiver;
    header->sender = m_node_id;
    memcpy(msg_buffer + SPHEADER_SIZE, buffer, size);

    uint8_t rval;
    if(receiver == 1){
        // message for BS
        rval = m_crypto.protectBufferForBSB(msg_buffer, SPHEADER_SIZE, &pLen);
    } else {
        // message for node
        rval = m_crypto.protectBufferForNodeB(receiver, msg_buffer, SPHEADER_SIZE, &pLen);
    }

    if(rval != SUCCESS){
        return rval;
    }

    uint8_t rf12_header = createHeader(receiver, MODE_DST, DEFAULT_REQ_ACK);
    rf12_sendNow(rf12_header, msg_buffer, pLen);

    return SUCCESS;
}

uint8_t ProtectLayer::sendToBS(msg_type_t msg_type, uint8_t *buffer, uint8_t size)
{
    if(size > MAX_MSG_SIZE - SPHEADER_SIZE){
        return FAIL;
    }

    if(msg_type == MSG_APP){
        return sendTo(msg_type, BS_NODE_ID, buffer, size);
    }

    if(msg_type != MSG_FORWARD){
        return FAIL;
    }

    uint8_t msg_buffer[MAX_MSG_SIZE];
    uint8_t msg_size = size + SPHEADER_SIZE;

    SPHeader_t *header = reinterpret_cast<SPHeader_t*>(msg_buffer);
    header->msgType = msg_type;
    header->sender = m_node_id;
    header->receiver = BS_NODE_ID;
    memcpy(msg_buffer + SPHEADER_SIZE, buffer, size);

    m_crypto.protectBufferForBSB(msg_buffer, SPHEADER_SIZE, &msg_size);
    
    return forwardToBS(msg_buffer, msg_size);
}

uint8_t ProtectLayer::forwardToBS(uint8_t *buffer, uint8_t size)
{
    SPHeader_t *header = reinterpret_cast<SPHeader_t*>(buffer);

    if(header->msgType != MSG_FORWARD){
        return FAIL;
    }

    // header->sender = m_node_id;
    // header->receiver = m_ctp->getParentID();
    uint8_t rf12_header = createHeader(m_ctp.getParentID(), MODE_DST, DEFAULT_REQ_ACK);
    rf12_sendNow(rf12_header, buffer, size);

    return SUCCESS;
}

uint8_t ProtectLayer::forwarduTESLA(uint8_t *buffer, uint8_t size)
{
    // does not work if they all resend it at once
    delay(m_node_id * 20);

    uint8_t rf12_header = createHeader(m_node_id, MODE_SRC, DEFAULT_REQ_ACK);
    rf12_sendNow(rf12_header, buffer, size);
    
    return SUCCESS;
}

uint8_t ProtectLayer::receive(uint8_t *buffer, uint8_t buff_size, uint8_t *received_size)
{
    return receive(buffer, buff_size, received_size, NODE_RECV_TIMEOUT_MS);
}

uint8_t ProtectLayer::receive(uint8_t *buffer, uint8_t buff_size, uint8_t *received_size, uint16_t timeout)
{
    if(!timeout){
        timeout = 1;
    }

    if(!waitReceive(millis() + timeout)){
        return ERR_TIMEOUT;
    }

    if(rf12_len > MAX_MSG_SIZE){
        return ERR_BUFFSIZE;
    }

    uint8_t rcvd_hdr;   // just to use macro copy_rf12_to_buffer
    uint8_t rcvd_len;
    uint8_t rcvd_buff[MAX_MSG_SIZE];

    copy_rf12_to_buffer();
    (void)rcvd_hdr; // unused

    SPHeader_t *header = reinterpret_cast<SPHeader_t*>(rcvd_buff);
    uint8_t rval;
    // if(m_node_id == 3){
    //     Serial.print("> ");                // TODO! REMOVE
    //     printBuffer(rcvd_buff, rcvd_len); // TODO! REMOVE
    // }

    if(header->msgType == MSG_FORWARD){
        if((rval = forwardToBS(rcvd_buff, rcvd_len)) == SUCCESS){
            return FORWARD;
        }
        return FAIL;
    }

    if(header->msgType == MSG_UTESLA){
        // if(m_utesla.verifyMessage(rcvd_buff + SPHEADER_SIZE, rcvd_len - SPHEADER_SIZE, false) != SUCCESS){
        //     return FAIL;
        // }

        // ignore already received message
        if(rcvd_buff[16] == m_received[0]){
            return FAIL;
        }
        m_received[0] = rcvd_buff[16];

        // not verifying anything, the key has not arrived yet
        // forward, return SUCCESS and verify in app

        if(forwarduTESLA(rcvd_buff, rcvd_len) != SUCCESS){
            return FAIL;
        }

        memcpy(buffer, rcvd_buff, rcvd_len);
        *received_size = rcvd_len;

        return SUCCESS;
    }

    if(header->msgType == MSG_UTESLA_KEY){
        if(rcvd_len  != SPHEADER_SIZE + m_mac.macSize()){
            return FAIL;
        }

        // ignore already received key
        if(rcvd_buff[16] == m_received[1]){
            return FAIL;
        }

        if(m_utesla.updateKey(rcvd_buff + SPHEADER_SIZE) != SUCCESS){
            return FAIL;
        }
        m_received[1] = rcvd_buff[16];

        if(forwarduTESLA(rcvd_buff, rcvd_len) != SUCCESS){
            return FAIL;
        }

        memcpy(buffer, rcvd_buff, rcvd_len);
        *received_size = rcvd_len;

        return FORWARD;
    }

    // TODO! handle uTESLA

    if(header->sender == BS_NODE_ID){
        rval = m_crypto.unprotectBufferFromBSB(rcvd_buff, SPHEADER_SIZE, &rcvd_len);
    } else {
        rval = m_crypto.unprotectBufferFromNodeB(header->sender, rcvd_buff, SPHEADER_SIZE, &rcvd_len);
    }

    if(rval != SUCCESS){
        return FAIL;
    }

    *received_size = rcvd_len - m_mac.macSize();

    if(*received_size > buff_size){
        return FAIL;
    }

    memcpy(buffer, rcvd_buff, *received_size);

    return SUCCESS;
}

uint8_t ProtectLayer::getNodeID()
{
    return m_node_id;
}

uint8_t ProtectLayer::verifyMessage(uint8_t *data, uint8_t data_size)
{
    return m_utesla.verifyMessage(data, data_size);
}

uint8_t ProtectLayer::neighborHandshake(uint8_t node_id)
{
    uint8_t msg_buffer[MAX_MSG_SIZE];
    uint8_t msg_size = SPHEADER_SIZE + 4;
    uint8_t rf12_header = createHeader(node_id, MODE_DST, DEFAULT_REQ_ACK);
    volatile SPHeader_t *spheader = reinterpret_cast<SPHeader_t*>(msg_buffer);

    spheader->msgType = MSG_DISC;
    spheader->sender = m_node_id;
    spheader->receiver = node_id;

    uint32_t nonce = random(/*0xFFFFFFFF*/);
    memcpy(msg_buffer + SPHEADER_SIZE, &nonce, 4);

    if(m_crypto.protectBufferForNodeB(node_id, msg_buffer, SPHEADER_SIZE, &msg_size) != SUCCESS){
        return FAIL;
    }

    rf12_sendNow(rf12_header, msg_buffer, msg_size);

    uint32_t start = millis();
    while(waitReceive(start + DISC_NEIGHBOR_RSP_TIME)){
        spheader = reinterpret_cast<volatile SPHeader_t*>(rf12_data);
        if(spheader->msgType != MSG_DISC || spheader->sender != node_id){
            continue;
        }

        memcpy(msg_buffer, rf12_data, rf12_len);
        msg_size = rf12_len;
        rf12_recvDone();

        if(m_crypto.unprotectBufferFromNodeB(node_id, msg_buffer, SPHEADER_SIZE, &msg_size) != SUCCESS){
            continue;
        }
        

        if(msg_size < SPHEADER_SIZE + 4){
            continue;
        }

        nonce++;
        if(memcmp(&nonce, msg_buffer + SPHEADER_SIZE, 4)){
            continue;
        }
        
        setBit(m_neighbors, node_id);
    }

    return SUCCESS;
}

uint8_t ProtectLayer::neighborHandshakeResponse()
{
    if(rf12_len < SPHEADER_SIZE + 4){
        return FAIL;
    }

    // data must already be in rf12_buff!!!!!
    uint8_t msg_buffer[MAX_MSG_SIZE];
    uint8_t msg_size = SPHEADER_SIZE + 4;

    memcpy(msg_buffer, rf12_data, rf12_len);
    msg_size = rf12_len;
    rf12_recvDone();

    SPHeader_t *spheader = reinterpret_cast<SPHeader_t*>(msg_buffer);

    if(spheader->msgType != MSG_DISC || spheader->receiver != m_node_id){
        return FAIL;
    }

    uint8_t other_id = spheader->sender;
    uint8_t rf12_header = createHeader(other_id, MODE_DST, DEFAULT_REQ_ACK);

    if(m_crypto.unprotectBufferFromNodeB(other_id, msg_buffer, SPHEADER_SIZE, &msg_size) != SUCCESS){
        return FAIL;
    }
    msg_size -= m_mac.macSize();

    uint32_t *nonce = reinterpret_cast<uint32_t*>(msg_buffer + SPHEADER_SIZE);
    (*nonce)++;

    spheader->sender = m_node_id;
    spheader->receiver = other_id;

    if(m_crypto.protectBufferForNodeB(other_id, msg_buffer, SPHEADER_SIZE, &msg_size) != SUCCESS){
        return FAIL;
    }

    rf12_sendNow(rf12_header, msg_buffer, msg_size);

    return SUCCESS;
}


uint8_t ProtectLayer::discoverNeighbors()
{
    randomSeed(analogRead(0));  // TODO better source of entropy
    uint32_t start = 0;

    // uint8_t msg_buffer[MAX_MSG_SIZE];
    uint8_t rf12_header = createHeader(m_node_id, MODE_SRC, DEFAULT_REQ_ACK);
    SPHeader_t spheader;// = reinterpret_cast<SPHeader_t*>(msg_buffer);

    spheader.msgType = MSG_DISC;
    spheader.sender = m_node_id;
    spheader.receiver = 0;

    delay(m_node_id * 10);    // pure heuristics

    // each bit means possible neighbor has/hasn't announce their presence
    uint32_t announced = 0;
    volatile SPHeader_t *rcvd_spheader = reinterpret_cast<volatile SPHeader_t*>(rf12_data);

    // broadcast ID in plaintext and mark nodes that messages came from
    for(int i=0;i<DISC_REBROADCASRS_NUM;i++){
        start = millis();
        // trying to randomize it a little so all the nodes do not broadcast at once
        while(waitReceive(start + m_node_id * 5)){
            if(rf12_len != 3 || rcvd_spheader->msgType != MSG_DISC){
                continue;
            }

            setBit(announced, rcvd_spheader->sender);
        }

        rf12_sendNow(rf12_header, &spheader, SPHEADER_SIZE);

        while(waitReceive(start + DISC_REBROADCASTS_DELAY)){
            if(rf12_len != 3 || rcvd_spheader->msgType != MSG_DISC){
                continue;
            }

            setBit(announced, rcvd_spheader->sender);
        }
    }

    for(int round=0;round<DISC_ROUNDS_NUM * 2;round++){
        int i = m_node_id + 1;
        while(i != m_node_id){
            if(bitIsSet(announced, i) && !(bitIsSet(m_neighbors, i))){
                // start handshake if even ID
                if((m_node_id + round) % 2){
                    if(neighborHandshake(i) != SUCCESS){
                        return FAIL;
                    }
                }
            }

            start = millis();
            while(waitReceive(start + 100)){
                neighborHandshakeResponse();
            }

            i = (i + 1) % (MAX_NODE_NUM + 1);
        }
    }

    return SUCCESS;
}

uint32_t ProtectLayer::getNeighbors()
{
    return m_neighbors;
}

#endif

