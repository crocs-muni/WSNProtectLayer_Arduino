/**
 * @brief uTESLA class for base station
 * 
 * @file uTESLAMaster.h
 * @author Martin Sarkany
 * @date 05/2018
 */

#ifndef UTESLAMASTER_H
#define UTESLAMASTER_H

#ifdef __linux__

#include <vector>
#include <string>

#include <stdint.h>

#include "ProtectLayerGlobals.h"

// sync window for uTESLA keys
#define MAX_NUM_MISSED_ROUNDS   5

class uTeslaMaster {
private:
    Hash                    *m_hash;                // class providing hash computation
    MAC                     *m_mac;                 // class providing MAC computation

    uint32_t                m_rounds_num;           // number of uTESLA rounds
    int32_t                 m_current_key_index;    // index of current key used
    std::vector<uint8_t*>   m_hash_chain;           // hash chain

    uint32_t                m_hash_size;            // hash size
    uint32_t                m_mac_size;             // MAC size
    uint32_t                m_mac_key_size;         // MAC key size

    int32_t                 m_dev_fd;               // file descriptor of slave device

    /**
     * @brief Broadcast uTESLA key for previous round
     * 
     * @return uint8_t SUCCESS or FAIL
     */
    uint8_t broadcastKey();
    
public:
    /**
     * @brief Constructor
     * 
     * @param device_fd     Open file descriptor of slave device
     * @param initial_key   First element of the hash chain
     * @param rounds_num    Number of uTESLA rounds
     * @param hash          Class providing hash interface
     * @param mac           Class providing MAC interface
     */
    uTeslaMaster(const int32_t device_fd, const uint8_t *initial_key, const uint32_t rounds_num, Hash *hash, MAC *mac);

    /**
     * @brief Destructor
     * 
     */
    virtual ~uTeslaMaster();

    /**
     * @brief Broadcast key and start a new round
     * 
     * @return uint8_t SUCCESS or FAIL
     */
    uint8_t newRound();

    /**
     * @brief Print the last hash chain element - usefull for debug purposses
     * 
     */
    void printLastElementHex();

    /**
     * @brief Broadcast message protected by MAC with current (unbroadcasted) key
     * 
     * @param data      Message
     * @param data_len  Message size
     * @return uint8_t  SUCCESS or FAIL
     */
    uint8_t broadcastMessage(const uint8_t* data, const uint16_t data_len);
};

/**
 * @brief Exception class for uTESLA class
 * 
 */
class uTeslaMasterException: public std::exception {
private:
    std::string m_msg;
public:
    uTeslaMasterException(const char *msg): m_msg(msg) {}
    std::string what(){ return m_msg; }
};

#endif  // __linux__

#endif // UTESLAMASTER_H
