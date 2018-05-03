// #undef __linux__ // TODO! REMOVE - for VS Code syntax highlighting
#ifndef __linux__

#include "uTESLAClient.h"

#include <RF12.h>
#include <avr/eeprom.h>

#include "common.h"

uTeslaClient::uTeslaClient(int8_t *eeprom_address, Hash *hash, MAC *mac): m_hash(hash), m_mac(mac), m_round(0)
{
	// for(int i=0;i<hash->hashSize();i++){
	// 	m_current_key[i] = EEPROM.read(eeprom_address + i);
	// }

    eeprom_read_block(m_current_key, eeprom_address, m_hash->hashSize());

	m_hash_size = hash->hashSize();
	m_mac_key_size = mac->keySize();
    m_mac_size = mac->macSize();
}

uTeslaClient::~uTeslaClient()
{
	memset(m_current_key, 0, m_hash_size);	// avr safe alternative?
}

int32_t uTeslaClient::getRoundNum()
{
    return m_round;
}

uint8_t uTeslaClient::getMacSize()
{
    return m_mac_size;
}

uint8_t uTeslaClient::getKeySize()
{
    return m_hash_size;
}

// TODO optimize!
uint8_t uTeslaClient::updateKey(const uint8_t* new_key)
{
    uint8_t *hash_prev = m_working_buffer;
    uint8_t *hash_next = m_working_buffer + 16;

	memcpy(hash_prev, new_key, m_hash_size);
	for(int i=0;i<MAX_NUM_MISSED_ROUNDS;i++){
		m_hash->hash(hash_prev, m_hash_size, hash_next, m_hash_size);
		if(!memcmp(m_current_key, hash_next, m_hash_size)){
            // m_round++;
            m_round += i + 1;
            memcpy(m_current_key, new_key, m_hash_size);
Serial.println("eq"); // TODO! REMOVE
			return SUCCESS;
		}

        // TODO! REMOVE
        printBuffer(new_key, 16);
        printBuffer(hash_next, 16);
        printBuffer(m_current_key, 16);
		
		memcpy(hash_prev, hash_next, m_hash_size);
	}

	return FAIL;
}

// uint8_t uTeslaClient::updateKey(const uint8_t* new_key)
// {
//     memset(m_working_buffer, 0, WORKING_BUFF_SIZE);
//     m_hash->hash(new_key, m_hash_size, m_working_buffer, m_hash_size);
    
//     // // TODO! REMOVE
//     // Serial.println();
//     // printBuffer(new_key, m_hash_size);
//     // printBuffer(m_current_key, m_hash_size);
//     // printBuffer(m_working_buffer, m_hash_size);
    
//     if(!memcmp(m_working_buffer, m_current_key, m_hash_size)){
//         memcpy(m_current_key, new_key, m_hash_size);
//         m_round++;
//         return SUCCESS;
//     }

//     return FAIL;
// }

uint8_t uTeslaClient::verifyMAC(const uint8_t* data, const uint16_t data_len, const uint8_t* mac)
{
    memset(m_working_buffer, 0, WORKING_BUFF_SIZE);

    m_mac->computeMAC(m_current_key, m_mac_key_size, data, data_len, m_working_buffer, m_mac_size);

    if(!memcmp(m_working_buffer, mac, m_mac_size)){
        return SUCCESS;
    }

	return FAIL;
}

uint8_t uTeslaClient::verifyMessage(const uint8_t *data, const uint8_t data_size)
{
    return verifyMAC(data, data_size - m_mac_size, data + data_size - m_mac_size);
}

#endif // __linux__