/**
 * @brief uTESLA class for non-BS devices
 * 
 * @file    uTESLAClient.h
 * @author  Martin Sarkany
 * @date    05/2018
 */

#ifndef UTESLACLIENT_H
#define UTESLACLIENT_H

#ifndef __linux__

#include <Arduino.h>
#include <EEPROM.h>
#include <RF12.h>

#include "ProtectLayerGlobals.h"

#define MAX_KEY_SIZE            16  // possible 128-bit output of hash function
#define MAX_MAC_SIZE            16  // possible 128-bit output of mac function
#define WORKING_BUFF_SIZE       32

#define MAX_NUM_MISSED_ROUNDS   5


/**
 * @brief Class providing uTESLA features for common (non-BS) nodes
 * 
 */
class uTeslaClient {
private:
    Hash            *m_hash;                            // hash class
    MAC             *m_mac;                             // MAC class
	uint8_t         m_current_key[MAX_KEY_SIZE];        // current key for message verification
    uint32_t        m_round;                            // index of a current round
    uint8_t         m_hash_size;                        // size of a hash function output
    uint8_t         m_mac_key_size;                     // size of a MAC function key
    uint8_t         m_mac_size;                         // size of a MAC function output
    uint8_t         m_working_buffer[WORKING_BUFF_SIZE];// buffer for different computations
    uint32_t        m_last_key_update;

public:
    /**
     * @brief Constructor
     * 
     * @param eeprom_address    Address of a key in EEPROM
     * @param hash              Hash class
     * @param mac               MAC class
     */
    uTeslaClient(int8_t *eeprom_address, Hash *hash, MAC *mac);

    /**
     * @brief Destructor
     * 
     */
    virtual ~uTeslaClient();

    /**
     * @brief Get index of current round
     * 
     * @return int32_t Index of current round
     */
    int32_t getRoundNum();

    /**
     * @brief Get size of MAC that is used
     * 
     * @return uint8_t Size of the MAC
     */
    uint8_t getMacSize();

    /**
     * @brief Get MAC key size
     * 
     * @return uint8_t Size of the MAC key
     */
    uint8_t getKeySize();

    /**
     * @brief Update key
     * 
     * @param new_key   New key to be verified and set
     * @return uint8_t  SUCCESS or FAIL
     */
    uint8_t updateKey(const uint8_t* new_key);

    /**
     * @brief Verify MAC of the data with current key
     * 
     * @param data      Data to be verified
     * @param data_len  Size of the data
     * @param mac       MAC of the data
     * @return uint8_t  SUCCESS or FAIL
     */
    uint8_t verifyMAC(const uint8_t* data, const uint16_t data_len, const uint8_t* mac);

    /**
     * @brief Verify MAC of the message (including header)
     * 
     * @param data      Message to be verified
     * @param data_size Message size including header and MAC
     * @return uint8_t  SUCCESS or FAIL
     */
    uint8_t verifyMessage(const uint8_t *data, const uint8_t data_size);

    /**
     * @brief Get time of the last key update
     * 
     * @return uint32_t Time of the last key update
     */
    uint32_t getLastKeyUpdate();
};

#endif // __linux__

#endif // UTESLACLIENT_H
