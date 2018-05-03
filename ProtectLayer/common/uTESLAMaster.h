#ifndef UTESLAMASTER_H
#define UTESLAMASTER_H

// TODO! move uTESLAMaster.* to common, delete BS_host

#ifdef __linux__

#include <vector>
#include <string>

#include <stdint.h>


//#include "blake224_crypto.h"
//#include "AES_crypto.h"
#include "uTESLA.h"

#ifndef MAX_KEY_SIZE
#define MAX_KEY_SIZE            32  // possible 256-bit output of hash function
#endif
#define AES_KEY_SIZE            16
#define AES_BLOCK_SIZE          16
#define BLAKE_HASH_SIZE         28
#define BLAKE_BLOCK_SIZE        64

#define MAX_NUM_MISSED_ROUNDS   5

void printBufferHex(const uint8_t *buffer, const uint32_t len);

class uTeslaMaster {
private:
    Hash                    *m_hash;
    MAC                     *m_mac;

    uint32_t                m_rounds_num;
    int32_t                 m_current_key_index;
    std::vector<uint8_t*>   m_hash_chain;

    uint32_t                m_hash_size;
    uint32_t                m_mac_size;
    uint32_t                m_mac_key_size;

    int32_t                 m_dev_fd;   // serial port to slave device


    void openSerialPort(std::string &serial_port);
    uint8_t broadcastKey();

public:
    uTeslaMaster(std::string &serial_port, const uint8_t *initial_key, const uint32_t rounds_num, Hash *hash, MAC *mac);
    uTeslaMaster(const int32_t device_fd, const uint8_t *initial_key, const uint32_t rounds_num, Hash *hash, MAC *mac);
    virtual ~uTeslaMaster();

    uint8_t newRound();

    void printLastElementHex();

    uint8_t broadcastMessage(const uint8_t* data, const uint16_t data_len);
};

class uTeslaMasterException: public std::exception {
private:
    std::string m_msg;
public:
    // uTeslaMasterException(std::string &msg): m_msg(msg) {}
    uTeslaMasterException(const char *msg): m_msg(msg) {}
    std::string what(){ return m_msg; }
};

#endif  // __linux__

#endif // UTESLAMASTER_H
