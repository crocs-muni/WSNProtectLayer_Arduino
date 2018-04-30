#include "ProtectLayer.h"


#ifdef __linux__
#include "configurator.h"

#include <termios.h>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>


// TODO! duplicate serial port opening - same code in configurator
// stolen from stackoverflow
int set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
            std::cerr << "error " << errno << " from tcgetattr" << std::endl;
            return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 10;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0){
            std::cerr << "error " << errno << " from tcsetattr: " << std::endl;
            return -1;
    }
    return 0;
}

void set_blocking (int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0){
            std::cerr << "error " << errno << " from tggetattr" << std::endl;
            return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 30;

    if (tcsetattr (fd, TCSANOW, &tty) != 0){
         std::cerr << "error " << errno << " setting term attributes" << std::endl;
    }
}

int openSerialPort(std::string path)
{
    int serial_fd = open(path.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd < 0){
        std::cerr << "Failed to open serial port " << path << ", errno: " << errno << std::endl;
        return serial_fd;
    }
    set_interface_attribs(serial_fd, B115200, 0);
    set_blocking(serial_fd, 0); // set not blocking

    return serial_fd;
}


ProtectLayer::ProtectLayer(std::string slave_path, std::string &key_file):
m_hash(&m_aes), m_mac(&m_aes), m_keydistrib(key_file), m_crypto(&m_aes, &m_mac, &m_hash, &m_keydistrib)
{
    int m_slave_fd = openSerialPort(slave_path);
    if(m_slave_fd < 0){
        throw std::runtime_error("Failed to open serial port");
    }
// Configurator(std::string &in_filename, const int uTESLA_rounds, const int key_size = 0);
    Configurator configurator(key_file, 0, 0);
    m_utesla = new uTeslaMaster(m_slave_fd, configurator.getuTESLAKey(), configurator.getuTESLARounds(), &m_hash, &m_mac);
}

ProtectLayer::~ProtectLayer()
{
    delete m_utesla;
    close(m_slave_fd);
}


uint8_t ProtectLayer::startCTP()
{
    m_ctp.startCTP(CTP_DURATION_MS);
}

uint8_t ProtectLayer::sendTo(msg_type_t msg_type, uint8_t receiver, uint8_t *buffer, uint8_t size)
{
    // TODO! not implemented yet
    // receiver == 1 is BS
}

uint8_t ProtectLayer::receive(uint8_t *buffer, uint8_t buff_size, uint8_t *received_size)
{
    if(read(m_slave_fd, buffer, buff_size) < 1){
        return FAIL;
    }

    // TODO! unprotect buffer

    return SUCCESS;
}


#else
#include "RF12.h"
#include <avr/eeprom.h>

#ifdef ENABLE_UTESLA
// initialize also uTESLA
ProtectLayer::ProtectLayer():
m_hash(&m_aes), m_mac(&m_aes), m_crypto(&m_aes, &m_mac, &m_hash, &m_keydistrib), m_utesla((int16_t) 0x1F4, &m_hash, &m_mac)
#else
// do not initialize uTESLA
ProtectLayer::ProtectLayer():
m_hash(&m_aes), m_mac(&m_aes), m_crypto(&m_aes, &m_mac, &m_hash, &m_keydistrib)
#endif
{
    Serial.begin(BAUD_RATE);

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
    return sendTo(msg_type, BS_NODE_ID, buffer, size);
}

uint8_t ProtectLayer::receive(uint8_t *buffer, uint8_t buff_size, uint8_t *received_size)
{
    // uint32_t now = millis();
    
    if(!waitReceive(millis() + NODE_RECV_TIMEOUT_MS)){
        return ERR_TIMEOUT;
    }

    if(rf12_len > MAX_MSG_SIZE){
        return ERR_BUFFSIZE;
    }

    uint8_t rcvd_hdr;
    uint8_t rcvd_len;
    uint8_t rcvd_buff[MAX_MSG_SIZE];

    if(rf12_len > buff_size){
        return FAIL;
    }
    copy_rf12_to_buffer();

    // TODO unprotect
    SPHeader_t *header = reinterpret_cast<SPHeader_t*>(rcvd_buff);
    uint8_t rval;
    Serial.print("> ");                // TODO! REMOVE
    printBuffer(rcvd_buff, rcvd_len); // TODO! REMOVE
    if(header->sender == BS_NODE_ID){
        rval = m_crypto.unprotectBufferFromBSB(rcvd_buff, SPHEADER_SIZE, &rcvd_len);
    } else {
        rval = m_crypto.unprotectBufferFromNodeB(header->sender, rcvd_buff, SPHEADER_SIZE, &rcvd_len);
    }

    if(rval != SUCCESS){
        return FAIL;
    }

#ifdef AUTOFORWARD
    // TODO protect buffer for parent and send
#endif

    memcpy(buffer, rcvd_buff, rcvd_len);
    *received_size = rcvd_len;

    return SUCCESS;
}


#endif

