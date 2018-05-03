#ifndef __linux__

#include "uTESLAClient.h"

#include <RF12.h>


uTeslaClient::uTeslaClient(int16_t eeprom_address, Hash *hash, MAC *mac): m_hash(hash), m_mac(mac), m_round(0)
{
	for(int i=0;i<hash->hashSize();i++){
		m_current_key[i] = EEPROM.read(eeprom_address + i);
	}

	m_hash_size = hash->hashSize();
	m_mac_key_size = mac->keySize();
    m_mac_size = mac->macSize();
}

uTeslaClient::uTeslaClient(const uint8_t* initial_key, Hash *hash, MAC *mac): m_hash(hash), m_mac(mac), m_round(0)
{
	memcpy(m_current_key, initial_key, hash->hashSize());
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
// bool uTeslaClient::updateKey(const uint8_t* new_key_hash)
// {
// 	//uint8_t hash_prev[m_hash_size]; // var len array
//     uint8_t *hash_prev = new uint8_t[m_hash_size];
// 	//uint8_t hash_next[m_hash_size]; // var len array
//     uint8_t *hash_next = new uint8_t[m_hash_size];

// 	memcpy(hash_prev, new_key_hash, m_hash_size);
// 	for(int i=0;i<MAX_NUM_MISSED_ROUNDS;i++){
// 		m_hash->hash(hash_prev, m_hash_size, hash_next, m_hash_size);
// 		if(!memcmp(m_current_key, hash_next, m_hash_size)){
//             delete[] hash_prev;
//             delete[] hash_next;
//             m_round++;

// 			return true;
// 		}
		
// 		memcpy(hash_prev, hash_next, m_hash_size);
// 	}

//     delete[] hash_prev;
//     delete[] hash_next;

// 	return false;
// }
#define DEBUG
bool uTeslaClient::updateKey(const uint8_t* new_key_hash)
{
    // uint8_t *hashed = new uint8_t[m_hash_size];

    memset(m_working_buffer, 0, WORKING_BUFF_SIZE);
    m_hash->hash(new_key_hash, m_hash_size, m_working_buffer, m_hash_size);
// #ifdef DEBUG
//     Serial.println();
//     printBuffer(new_key_hash, m_hash_size);
//     printBuffer(m_current_key, m_hash_size);
//     printBuffer(m_working_buffer, m_hash_size);
// #endif // DEBUG


    if(!memcmp(m_working_buffer, m_current_key, m_hash_size)){
        memcpy(m_current_key, new_key_hash, m_hash_size);
        m_round++;
        // delete[] hashed;
        return true;
    }

    // delete[] hashed;

    return false;
}

bool uTeslaClient::verifyMAC(const uint8_t* data, const uint16_t data_len, const uint8_t* mac)
{
    // uint8_t computed_mac[m_mac_size];   // var len array
    // uint8_t *computed_mac = new uint8_t[m_mac_size];
    // uint8_t computed_mac[MAX_MAC_SIZE];
    memset(m_working_buffer, 0, WORKING_BUFF_SIZE);

    // printBuffer(m_current_key, m_hash_size);
    // printBuffer(data, data_len);
    m_mac->computeMAC(m_current_key, m_mac_key_size, data, data_len, m_working_buffer, m_mac_size);
    
    
    // Serial.println("MAC comp: ");
    // printBuffer(mac, m_mac_size);
    // printBuffer(computed_mac, m_mac_size);

    if(!memcmp(m_working_buffer, mac, m_mac_size)){
        // delete[] computed_mac;
        return true;
    }

    // delete[] computed_mac;

	return false;
}

bool uTeslaClient::verifyMessage(const uint8_t *data, const uint8_t data_size, uint8_t includes_header)
{
    if(includes_header){
        const SPHeader_t *spheader = reinterpret_cast<const SPHeader_t*>(data);
        if(spheader->msgType != MSG_UTESLA || spheader->sender != BS_NODE_ID){
            return false;
        }

        return verifyMAC(data + SPHEADER_SIZE, data_size - SPHEADER_SIZE - m_mac_size, data + data_size - m_mac_size);
    }

    return verifyMAC(data, data_size - m_mac_size, data + data_size - m_mac_size);
}

#endif // __linux__