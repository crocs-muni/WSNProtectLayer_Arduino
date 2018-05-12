/**
 * @brief Definitions, and functions common for whole project. To be merged with ProtectLayerGlobals.h
 * 
 * @file    common.h
 * @author  Martin Sarkany
 * @date    05/2018
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef __linux__
#include <Arduino.h>
#include <RF12.h>
#endif
#include <stdint.h>


#define FAIL                1           // return value indicating failure
#define SUCCESS             0           // return value indicating success

// radio settings
#define BAUD_RATE           115200      // serial port baud rate
#define RADIO_FREQ          RF12_868MHZ // RF12 radio frequency
#define RADIO_GROUP         10          // RF12 radio group

// EEPROM settings
#define NODE_ID_LOCATION    0           // node ID EEPROM address
#define MAX_NODE_NUM        29          // maximum number of nodes

// createHeader() modes
#define MODE_SRC            0           // include source address
#define MODE_DST            1           // include destination address

#define ERR_OK              0           // everything ok, not an error
#define ERR_NULLARG         1           // NULL where it shouldn't be
#define ERR_INVARG          2           // invalid argument
#define ERR_MSG_ADD         3           // failed to add message
#define ERR_KEY_UPDT        4           // failed to update key
#define ERR_MSG_VRF         5           // message verification failed
#define ERR_MSG_SIZE        6           // wrong message size
#define ERR_SERIAL_RD       7           // failed to read message from serial port
#define ERR_BUFFSIZE        8           // buffer too small
#define ERR_TIMEOUT         9           // timeout

// requires rcvd_len, rcvd_hdr, rcvd_buff variables
#define copy_rf12_to_buffer() { rcvd_len = rf12_len; rcvd_hdr = rf12_hdr; memcpy(rcvd_buff, (const void*) rf12_data, rf12_len); replyAck(); rf12_recvDone(); }


#ifdef  __linux__

/**
 * @brief Print message if DEBUG is defined
 * 
 * @param msg       Message to be printed
 * @param is_error  Message is an error
 */
void printDebug(const char *msg, bool is_error = false);

/**
 * @brief Print buffer in hex format
 * 
 * @param buffer    Buffer to be printed
 * @param len       Buffer size
 */
void printBufferHex(const uint8_t *buffer, const uint32_t len);

/**
 * @brief Number of millisenconds since program start
 * 
 * @return uint64_t Number of millisenconds since program start
 */
uint64_t millis();

#else

/**
 * @brief Send acknowledgement if required
 * 
 */
void replyAck();

/**
 * @brief Create RF12 header
 * 
 * @param id            Source or destination node ID
 * @param mode          MODE_SRC or MODE_DST
 * @param requireACK    Message requires ackowledgement
 * @return uint8_t      Created header
 */
uint8_t createHeader(uint8_t id, uint8_t mode, bool requireACK);

/**
 * @brief Print buffer through serial port
 * 
 * @param buffer    Buffer to be printed
 * @param len       Buffer size
 */
void printBuffer(const uint8_t *buffer, const uint8_t len);

/**
 * @brief Blocking receive. Terminates when a packet is received or a device is running for 'end' milliseconds
 * 
 * @param end   Termination time
 * @return true if packet was received or false otherwise
 */
bool waitReceive(uint32_t end);

/**
 * @brief Send error to a Linux host
 * 
 * @param err_num   Error code
 */
void printError(int err_num);

/**
 * @brief Get amount of availbale RAM
 * 
 * @return int amount of availbale RAM
 */
int freeRam();

#endif // __linux__

#endif // _COMMON_H_
