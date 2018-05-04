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

class uTeslaClient {
private:
    Hash            *m_hash;
    MAC             *m_mac;
	uint8_t         m_current_key[MAX_KEY_SIZE];
    uint32_t        m_round;
    uint8_t         m_hash_size;
    uint8_t         m_mac_key_size;
    uint8_t         m_mac_size;
    uint8_t         m_working_buffer[WORKING_BUFF_SIZE];

public:
    uTeslaClient(int8_t *eeprom_address, Hash *hash, MAC *mac);
    // uTeslaClient(int16_t eeprom_address, Hash *hash, MAC *mac);
    // uTeslaClient(const uint8_t* initial_key, Hash *hash, MAC *mac);
    virtual ~uTeslaClient();

    int32_t getRoundNum();

    uint8_t getMacSize();

    uint8_t getKeySize();

    uint8_t updateKey(const uint8_t* new_key);

    uint8_t verifyMAC(const uint8_t* data, const uint16_t data_len, const uint8_t* mac);

    uint8_t verifyMessage(const uint8_t *data, const uint8_t data_size);
};

#endif // __linux__

#endif // UTESLACLIENT_H
