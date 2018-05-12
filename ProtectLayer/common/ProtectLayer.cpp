/**
 * @brief Implementation of ProtectLayer for Arduino-based JeeLink devices featuring RF12 radio
 * 
 * @file ProtectLayer.cpp
 * @author Martin Sarkany
 * @date 05/2018
 */

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
m_hash(&m_aes), m_mac(&m_aes), m_keydistrib(key_file), m_crypto(&m_aes, &m_mac, &m_hash, &m_keydistrib)
{ 
    // open file descriptor for serial port
    m_slave_fd = openSerialPort(slave_path);
    if(m_slave_fd < 0){
        throw std::runtime_error("Failed to open serial port");
    }

#ifdef ENABLE_CTP
    // set file descriptor in CTP class
    m_ctp.setSlaveFD(m_slave_fd);
#endif
    // initialize configurator class to read keys from the file
    Configurator configurator(key_file, 0, 0);

    // initialize uTESLA class with keys from configurator
    m_utesla = new uTeslaMaster(m_slave_fd, configurator.getuTESLAKey(), configurator.getuTESLARounds(), &m_hash, &m_mac);

    // fire up the device - sometimes it takes a read first
    uint8_t buffer[MAX_MSG_SIZE];
    read(m_slave_fd, buffer, MAX_MSG_SIZE);
}

ProtectLayer::~ProtectLayer()
{
    // uTESLA was dynamically allocated
    delete m_utesla;
    
    // close slave device if open
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
    // return FAIL in case of too long messages, NULL buffer or invalid recipient
    if(size > MAX_MSG_SIZE + SPHEADER_SIZE + m_mac.macSize() || receiver < 2 || receiver > MAX_NODE_NUM || !buffer){
        return FAIL;
    }
    
    // return FAIL if not allowed message type
    if(msg_type != MSG_OTHER && msg_type != MSG_APP){
        return FAIL;
    }
    
    uint8_t msg_buffer[MAX_MSG_SIZE + 2];

    // set header pointer to beginning of the message (first 2 bytes are only for serial communication between master and slave BS devices)
    SPHeader_t *spheader = reinterpret_cast<SPHeader_t*>(msg_buffer + 2);
    // set the header
    spheader->msgType = msg_type;
    spheader->sender = BS_NODE_ID;
    spheader->receiver = receiver;

    // copy the rest of the message into buffer
    memcpy(msg_buffer + 2 + SPHEADER_SIZE, buffer, size);

    // set the new size to size + header size
    size += SPHEADER_SIZE;
    // encryption and MAC
    if(m_crypto.protectBufferForNodeB(receiver, msg_buffer + 2, SPHEADER_SIZE, &size) != SUCCESS){
        return FAIL;
    }

    // set first 2 bytes for serial communication
    msg_buffer[0] = size;
    msg_buffer[1] = size;

    // send buffer to device
    int wr_len = 0;
    if((wr_len = write(m_slave_fd, buffer, size + 2)) != size + 2){
        return FAIL;
    }

    // receive the response
    int rd_len = 0;
    if((rd_len = read(m_slave_fd, buffer, MAX_MSG_SIZE)) != 1){
        return FAIL;
    }

    // check the response
    if(buffer[0] != ERR_OK){
        return FAIL;
    }

    return SUCCESS;
}

uint8_t ProtectLayer::receive(uint8_t *buffer, uint8_t buff_size, uint8_t *received_size)
{
    uint8_t rcvd_len = 0;
    uint8_t rcvd_buff[MAX_MSG_SIZE + 10];

    // get new message from the slave
    if((rcvd_len = read(m_slave_fd, (void*) rcvd_buff, MAX_MSG_SIZE)) < SPHEADER_SIZE + m_mac.macSize()){
        // return FAIL if there is none
        return FAIL;
    }

    // discard message if it does not fit into buffer
    if(rcvd_len > MAX_MSG_SIZE){    // cannot happen
        return ERR_BUFFSIZE;
    }

    // set the header pointer
    SPHeader_t *spheader = reinterpret_cast<SPHeader_t*>(rcvd_buff);
    uint8_t rval;

    // discard messages for other nodes
    if(spheader->receiver != BS_NODE_ID){
        return FAIL;
    }

    // decrypt and verify MAC
    if((rval = m_crypto.unprotectBufferFromNodeB(spheader->sender, rcvd_buff, (uint8_t) SPHEADER_SIZE, &rcvd_len)) != SUCCESS){
        return FAIL;
    }

    // set the actual data size + header
    *received_size = rcvd_len - m_mac.macSize();

    // return FAIL if plaintext does not fit into buffer
    if(*received_size > buff_size){
        return FAIL;
    }

    // copy message into buffer
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
m_hash(&m_aes), m_mac(&m_aes), m_keydistrib(&m_neighbors), m_crypto(&m_aes, &m_mac, &m_hash, &m_keydistrib), m_neighbors(0), m_utesla(UTESLA_KEY_ADDRESS, &m_hash, &m_mac)
#else
// do not initialize uTESLA
ProtectLayer::ProtectLayer():
m_hash(&m_aes), m_mac(&m_aes), m_keydistrib(&m_neighbors), m_crypto(&m_aes, &m_mac, &m_hash, &m_keydistrib)
#endif
{
    // initialize serial communication
    Serial.begin(BAUD_RATE);

    // set received indicators to 0
    memset(m_received, 0, 2);

    // read the node ID from EEPROM
    m_node_id = eeprom_read_byte(0);

#ifdef ENABLE_CTP
    // set node ID to CTP class if enabled
    m_ctp.setNodeID(m_node_id);
#endif // ENABLE_CTP

    // initialize the radio
    rf12_initialize(m_node_id, RADIO_FREQ, RADIO_GROUP);
}

#ifdef ENABLE_CTP
uint8_t ProtectLayer::startCTP()
{
    return m_ctp.startCTP(CTP_DURATION_MS);
}

uint8_t ProtectLayer::sendCTP(msg_type_t msg_type, uint8_t *buffer, uint8_t size)
{
    return sendTo(msg_type, m_ctp.getParentID(), buffer, size);
}
#endif // ENABLE_CTP

uint8_t ProtectLayer::sendTo(msg_type_t msg_type, uint8_t receiver, uint8_t *buffer, uint8_t size)
{
    // return FAIL if NULL buffer or the message too long
    if(!buffer || size + SPHEADER_SIZE + m_mac.macSize() > MAX_MSG_SIZE){
        return FAIL;
    }

    // return FAIL if invalid receiver
    if(receiver < 1 || receiver > MAX_NODE_NUM){
        return FAIL;
    }

    // new message length - including header (will further increase when MAC is applied)
    uint8_t pLen = size + SPHEADER_SIZE;
    uint8_t msg_buffer[MAX_MSG_SIZE];   // TODO use some common buffer
    // set header pointer to beginning of the message
    SPHeader_t *header = reinterpret_cast<SPHeader_t*>(msg_buffer);
    memset(msg_buffer, 0, MAX_MSG_SIZE);

    // set the header
    header->msgType = msg_type;
    header->receiver = receiver;
    header->sender = m_node_id;
    memcpy(msg_buffer + SPHEADER_SIZE, buffer, size);

    uint8_t rval;
    // encryption and MAC
    if(receiver == BS_NODE_ID){
        // message for BS
        rval = m_crypto.protectBufferForBSB(msg_buffer, SPHEADER_SIZE, &pLen);
    } else {
        // message for node
        rval = m_crypto.protectBufferForNodeB(receiver, msg_buffer, SPHEADER_SIZE, &pLen);
    }

    // return on failure
    if(rval != SUCCESS){
        return rval;
    }

    // send over radio
    uint8_t rf12_header = createHeader(receiver, MODE_DST, DEFAULT_REQ_ACK);
    rf12_sendNow(rf12_header, msg_buffer, pLen);

    return SUCCESS;
}

uint8_t ProtectLayer::sendToBS(msg_type_t msg_type, uint8_t *buffer, uint8_t size)
{
    // return FAIL if NULL buffer or the message too long
    if(!buffer || size + SPHEADER_SIZE + m_mac.macSize() > MAX_MSG_SIZE){
        return FAIL;
    }

    // send one-hop message
    if(msg_type == MSG_APP || msg_type == MSG_OTHER){
        return sendTo(msg_type, BS_NODE_ID, buffer, size);
    }

#ifdef ENABLE_CTP
    if(msg_type != MSG_FORWARD){
        return FAIL;
    }

    // send message to be forwarded to BS
    uint8_t msg_buffer[MAX_MSG_SIZE];
    uint8_t msg_size = size + SPHEADER_SIZE;

    // set the header pointer
    SPHeader_t *header = reinterpret_cast<SPHeader_t*>(msg_buffer);
    header->msgType = msg_type;
    header->sender = m_node_id;
    header->receiver = BS_NODE_ID;
    memcpy(msg_buffer + SPHEADER_SIZE, buffer, size);

    // encryption and MAC
    m_crypto.protectBufferForBSB(msg_buffer, SPHEADER_SIZE, &msg_size);
    
    // send to a CTP parrent node
    return forwardToBS(msg_buffer, msg_size);
#else
    return FAIL;
#endif // ENABLE_CTP

}

#ifdef ENABLE_CTP
uint8_t ProtectLayer::forwardToBS(uint8_t *buffer, uint8_t size)
{
    SPHeader_t *header = reinterpret_cast<SPHeader_t*>(buffer);

    // check the message type to be sure it must be forwarded
    if(header->msgType != MSG_FORWARD){
        return FAIL;
    }

    // check if the parent is known
    if(m_ctp.getParentID() < 1 || m_ctp.getParentID() > MAX_NODE_NUM){
        return FAIL;
    }

    // forward to a CTP parent
    uint8_t rf12_header = createHeader(m_ctp.getParentID(), MODE_DST, DEFAULT_REQ_ACK);
    rf12_sendNow(rf12_header, buffer, size);

    return SUCCESS;
}
#endif // ENABLE_CTP

uint8_t ProtectLayer::forwarduTESLA(uint8_t *buffer, uint8_t size)
{
    // does not work if they all resend it at once
    delay(m_node_id * 20);

    // send
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
    // set timeout to 1 ms so it checks for new message at least once
    if(!timeout){
        timeout = 1;
    }

    // blocking receive
    if(!waitReceive(millis() + timeout)){
        // return ERR_TIMEOUT;
        return FAIL;
    }

    // return if the mesage does not fit into a buffer
    if(rf12_len > MAX_MSG_SIZE){
        // return ERR_BUFFSIZE;
        return FAIL;
    }

    uint8_t rcvd_hdr;   // just to use macro copy_rf12_to_buffer
    uint8_t rcvd_len;
    uint8_t rcvd_buff[MAX_MSG_SIZE];

    // copy to local variables and send acknowledgement if required
    copy_rf12_to_buffer();
    (void)rcvd_hdr; // unused

    // set header pointer
    SPHeader_t *header = reinterpret_cast<SPHeader_t*>(rcvd_buff);
    uint8_t rval;

#ifdef ENABLE_CTP
    // forward message if CTP is enabled
    if(header->msgType == MSG_FORWARD){
        if((rval = forwardToBS(rcvd_buff, rcvd_len)) == SUCCESS){
            return FORWARD;
        }
        return FAIL;
    }
#endif // ENABLE_CTP

#ifdef ENABLE_UTESLA
    if(header->msgType == MSG_UTESLA){
        // ignore messages if the key is already invalid
        if(millis() - m_utesla.getLastKeyUpdate() > UTESLA_KEY_VALID_PERIOD){
            return FAIL;
        }

        // ignore already received message
        // TODO use a sequence number or something like that - this will not work in some cases
        if(rcvd_buff[rcvd_len - 5] == m_received[0]){
            return FAIL;
        }
        m_received[0] = rcvd_buff[rcvd_len - 5];

        // not verifying anything, the key has not arrived yet
        // forward, return SUCCESS and verify in app
        if(forwarduTESLA(rcvd_buff, rcvd_len) != SUCCESS){
            return FAIL;
        }

        if(buff_size < rcvd_len){
            return FAIL;
        }

        memcpy(buffer, rcvd_buff, rcvd_len);
        *received_size = rcvd_len;

        return SUCCESS;
    }

    if(header->msgType == MSG_UTESLA_KEY){
        // check the size
        if(rcvd_len  != SPHEADER_SIZE + m_mac.macSize()){
            return FAIL;
        }

        // ignore already received key
        // TODO use a sequence number or something like that - this will not work in some cases
        if(rcvd_buff[16] == m_received[1]){
            return FAIL;
        }

        // update uTESLA key
        if(m_utesla.updateKey(rcvd_buff + SPHEADER_SIZE) != SUCCESS){
            return FAIL;
        }
        m_received[1] = rcvd_buff[16];

        // forward the key
        if(forwarduTESLA(rcvd_buff, rcvd_len) != SUCCESS){
            return FAIL;
        }

        // copy message to buffer - probably not needed but anyway..
        if(buff_size < rcvd_len){
            return FAIL;
        }

        memcpy(buffer, rcvd_buff, rcvd_len);
        *received_size = rcvd_len;

        // return different value than SUCCESS so the app can skip it easily
        return FORWARD;
    }
#endif // ENABLE_UTESLA
    if(header->msgType == MSG_DISC){
        if(neighborHandshakeResponse(rcvd_buff, rcvd_len) == SUCCESS){
            return HANDSHAKE;
        }

        return FAIL;
        // return FAIL if the session key has not been established and this is not a handshake message
    } else if(!(bitIsSet(m_neighbors, header->sender))){
        return FAIL;
    }

    // decrypt and verify MAC
    if(header->sender == BS_NODE_ID){
        rval = m_crypto.unprotectBufferFromBSB(rcvd_buff, SPHEADER_SIZE, &rcvd_len);
    } else {
        rval = m_crypto.unprotectBufferFromNodeB(header->sender, rcvd_buff, SPHEADER_SIZE, &rcvd_len);
    }
    if(rval != SUCCESS){
        return FAIL;
    }

    // set the size
    *received_size = rcvd_len - m_mac.macSize();

    // return FAIL if it does not fit into buffer
    if(*received_size > buff_size){
        return FAIL;
    }

    // copy to buffer
    memcpy(buffer, rcvd_buff, *received_size);

    return SUCCESS;
}

uint8_t ProtectLayer::getNodeID()
{
    return m_node_id;
}

#ifdef ENABLE_UTESLA
uint8_t ProtectLayer::verifyMessage(uint8_t *data, uint8_t data_size)
{
    return m_utesla.verifyMessage(data, data_size);
}
#endif // ENABLE_UTESLA

uint8_t ProtectLayer::neighborHandshake(uint8_t node_id)
{
    uint8_t random_buffer[AES_KEY_SIZE];
    uint8_t msg_buffer[MAX_MSG_SIZE];
    volatile SPHeader_t *spheader = reinterpret_cast<SPHeader_t*>(msg_buffer);

    spheader->msgType = MSG_DISC;
    spheader->sender = m_node_id;
    spheader->receiver = node_id;

    // create some nonce
    uint32_t own_nonce = random();
    uint32_t other_nonce = 0;
    *((uint32_t*)(msg_buffer + SPHEADER_SIZE)) = own_nonce;

    // create header
    uint8_t rf12_header = createHeader(node_id, MODE_DST, DEFAULT_REQ_ACK);

    // send nonce to node
    rf12_sendNow(rf12_header, &msg_buffer, sizeof(uint32_t) + SPHEADER_SIZE);

    uint32_t start = millis();
    while(waitReceive(start + DISC_NEIGHBOR_RSP_TIME)){
        // check the header
        spheader = reinterpret_cast<volatile SPHeader_t*>(rf12_data);
        if(spheader->msgType != MSG_DISC || spheader->sender != node_id){
            continue;
        }

        // copy the message
        memcpy(msg_buffer, rf12_data, rf12_len);
        uint8_t msg_size = rf12_len;
        rf12_recvDone();

        if(m_crypto.unprotectBufferFromNodeB(node_id, msg_buffer, SPHEADER_SIZE, &msg_size) != SUCCESS){
            return FAIL;
        }

        if(*((uint32_t*)(msg_buffer + SPHEADER_SIZE + (3 * sizeof(uint32_t)))) != own_nonce){
            return FAIL;
        }

        other_nonce = *((uint32_t*)(msg_buffer + SPHEADER_SIZE + (2 * sizeof(uint32_t))));
        memcpy(random_buffer, msg_buffer + SPHEADER_SIZE, 2 * sizeof(uint32_t));

        *((uint32_t*)(random_buffer + 2 * sizeof(uint32_t))) = random();
        *((uint32_t*)(random_buffer + 3 * sizeof(uint32_t))) = random();

        spheader = reinterpret_cast<volatile SPHeader_t*>(msg_buffer);
        spheader->sender = m_node_id;
        spheader->receiver = node_id;
        memcpy(msg_buffer + SPHEADER_SIZE, random_buffer + (2 * sizeof(uint32_t)), 2 * sizeof(uint32_t));
        memcpy(msg_buffer + SPHEADER_SIZE + (2 * sizeof(uint32_t)), &own_nonce, sizeof(uint32_t));
        memcpy(msg_buffer + SPHEADER_SIZE + (3 * sizeof(uint32_t)), &other_nonce, sizeof(uint32_t));
        msg_size = SPHEADER_SIZE + (4 * sizeof(uint32_t));

        if(m_crypto.protectBufferForNodeB(node_id, msg_buffer, SPHEADER_SIZE, &msg_size) != SUCCESS){
            return FAIL;
        }

        rf12_sendNow(rf12_header, msg_buffer, msg_size);

        while(waitReceive(start + DISC_NEIGHBOR_RSP_TIME)){
            // check the header
            spheader = reinterpret_cast<volatile SPHeader_t*>(rf12_data);
            if(spheader->msgType != MSG_DISC || spheader->sender != node_id){
                continue;
            }

            // copy the message
            memcpy(msg_buffer, rf12_data, rf12_len);
            msg_size = rf12_len;
            rf12_recvDone();

            if(msg_size < SPHEADER_SIZE + sizeof(uint32_t) + AES_MAC_SIZE){
                return FAIL;
            }

            if(m_crypto.unprotectBufferFromNodeB(node_id, msg_buffer, SPHEADER_SIZE, &msg_size) != SUCCESS){
                return FAIL;
            }

            if(memcmp(msg_buffer + SPHEADER_SIZE, &own_nonce, sizeof(uint32_t))){
                return FAIL;
            }

            if(m_keydistrib.deriveKeyToNode(node_id, random_buffer, 4 * sizeof(uint32_t), &m_mac) != SUCCESS){
                return FAIL;
            }

            // set bit in neighbors list
            setBit(m_neighbors, node_id);

            return SUCCESS;
        }
    }

    return FAIL;
}

uint8_t ProtectLayer::neighborHandshakeResponse(uint8_t *msg_buffer, uint8_t msg_size)
{
    // check the message size
    if(msg_size < SPHEADER_SIZE + 4){
        return FAIL;
    }

    uint8_t random_buffer[AES_KEY_SIZE];

    // check the header
    volatile SPHeader_t *spheader = reinterpret_cast<volatile SPHeader_t*>(msg_buffer);

    if(spheader->msgType != MSG_DISC || spheader->receiver != m_node_id){
        return FAIL;
    }

    // Serial.println("r");
    // printBuffer(msg_buffer, msg_size);

    uint8_t other_id = spheader->sender;
    uint8_t rf12_header = createHeader(other_id, MODE_DST, DEFAULT_REQ_ACK);

    uint32_t other_nonce = *((uint32_t*)(msg_buffer + SPHEADER_SIZE));
    uint32_t own_nonce = random();
    *((uint32_t*)(msg_buffer + SPHEADER_SIZE)) = random();
    *((uint32_t*)(msg_buffer + SPHEADER_SIZE + sizeof(uint32_t))) = random();
    
    memcpy(random_buffer, msg_buffer + SPHEADER_SIZE, 2 * sizeof(uint32_t));

    memcpy(msg_buffer + SPHEADER_SIZE + (2 * sizeof(uint32_t)), &own_nonce, sizeof(uint32_t));
    *((uint32_t*)(msg_buffer + SPHEADER_SIZE + (3 * sizeof(uint32_t)))) = other_nonce;
    msg_size = SPHEADER_SIZE + (4 * sizeof(uint32_t));
    spheader->sender = m_node_id;
    spheader->receiver = other_id;

    if(m_crypto.protectBufferForNodeB(other_id, msg_buffer, SPHEADER_SIZE, &msg_size) != SUCCESS){\
        return FAIL;
    }

    rf12_sendNow(rf12_header, msg_buffer, msg_size);

    uint32_t start = millis();
    while(waitReceive(start + DISC_NEIGHBOR_RSP_TIME)){
        // check the header
        spheader = reinterpret_cast<volatile SPHeader_t*>(rf12_data);
        if(spheader->msgType != MSG_DISC || spheader->sender != other_id){
            continue;
        }

        // copy the message
        memcpy(msg_buffer, rf12_data, rf12_len);
        msg_size = rf12_len;
        rf12_recvDone();

        if(m_crypto.unprotectBufferFromNodeB(other_id, msg_buffer, SPHEADER_SIZE, &msg_size) != SUCCESS){
            return FAIL;
        }

        if(*((uint32_t*)(msg_buffer + SPHEADER_SIZE + (3 * sizeof(uint32_t)))) != own_nonce){
            return FAIL;
        }

        if(*((uint32_t*)(msg_buffer + SPHEADER_SIZE + (2 * sizeof(uint32_t)))) != other_nonce){
            return FAIL;
        }
        memcpy(random_buffer + (2 * sizeof(uint32_t)), msg_buffer + SPHEADER_SIZE, 2 * sizeof(uint32_t));

        spheader = reinterpret_cast<volatile SPHeader_t*>(msg_buffer);
        spheader->sender = m_node_id;
        spheader->receiver = other_id;
        memcpy(msg_buffer + SPHEADER_SIZE, &other_nonce,sizeof(uint32_t));
        msg_size = SPHEADER_SIZE + sizeof(uint32_t);

        if(m_crypto.protectBufferForNodeB(other_id, msg_buffer, SPHEADER_SIZE, &msg_size) != SUCCESS){
            return FAIL;
        }

        if(m_keydistrib.deriveKeyToNode(other_id, random_buffer, 4 * sizeof(uint32_t), &m_mac) != SUCCESS){
            return FAIL;
        }

        // send
        rf12_sendNow(rf12_header, msg_buffer, msg_size);

        setBit(m_neighbors, other_id);

        return SUCCESS;

    }

    return FAIL;
}


uint8_t ProtectLayer::discoverNeighbors()
{
    // seed the PRNG
    randomSeed(analogRead(0));  // TODO better source of entropy

    uint32_t nodes_list = m_keydistrib.getNodesList();

    // start handshakes in few rounds
    for(int round=0;round<DISC_ROUNDS_NUM * 2;round++){
        // start with the node with next ID
        int i = m_node_id + 1;
        // loop through all IDs
        while(i != m_node_id){
            // perform handshake if the node exists
            if(bitIsSet(nodes_list, i) && !(bitIsSet(m_neighbors, i))){
                // start handshake if even ID
                if((m_node_id + round) % 2){
                    // if(neighborHandshake(i) != SUCCESS){
                    //     continue;
                    // }
                    neighborHandshake(i);
                }
            }

            // receive() automatically responds to handshake request
            uint8_t foo;
            // passing 0 buffer size in case other message arrives
            receive(&foo, 0, &foo, random(1200));

            i = (i + 1) % (MAX_NODE_NUM + 1);
        }
    }

#ifdef DELETE_KEYS
    // delete keys of other nodes
    for(int i=MIN_NODE_ID;i<MAX_NODE_NUM;i++){
        if(!(bitIsSet(m_neighbors, i))){
            m_keydistrib.deleteKey(i);
        }
    }
#endif // DELETE_KEYS

    return SUCCESS;
}

uint32_t ProtectLayer::getNeighbors()
{
    return m_neighbors;
}

#endif

