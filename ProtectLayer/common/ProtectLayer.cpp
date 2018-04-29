#include "ProtectLayer.h"


// #undef __linux__

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

uint8_t ProtectLayer::send(msg_type_t msg_type, uint8_t *buffer, uint8_t size)
{

}

uint8_t ProtectLayer::receive(uint8_t *buffer, uint8_t buff_size)
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

    m_node_id == eeprom_read_byte(0);
    m_ctp.setNodeID(m_node_id);

    rf12_initialize(m_node_id, RADIO_FREQ, RADIO_GROUP);
}


uint8_t ProtectLayer::startCTP()
{
    m_ctp.startCTP(CTP_DURATION_MS);
}

uint8_t ProtectLayer::send(msg_type_t msg_type, uint8_t *buffer, uint8_t size)
{
    // // TODO! fill m_msg_buffer

    // m_ctp.send(m_msg_buffer, )
}

uint8_t ProtectLayer::receive(uint8_t *buffer, uint8_t buff_size)
{
    // TODO!
}


#endif

