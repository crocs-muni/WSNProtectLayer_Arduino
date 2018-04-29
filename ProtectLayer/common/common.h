#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef __linux__
#include <Arduino.h>
#include <RF12.h>
#endif
#include <stdint.h>

typedef uint16_t node_id_t;


#define FAIL            1
#define SUCCESS         0

// radio settings
#define BAUD_RATE           115200
#define RADIO_FREQ          RF12_868MHZ
#define RADIO_GROUP         10

// EEPROM settings
#define NODE_ID_LOCATION    0
#define GROUP_ID_LOCATION   1
#define PARENT_ID_LOCATION  2

// buffer size settings
#define MSG_BUF_SIZE        40

// createHeader() and send_sybil_message() modes
#define MODE_SRC            0
#define MODE_DST            1

// devices' IDs settings
#define BS_ID               1

#define ERROR_MESSAGE       "Err"
#define ERR_OK              0   // everything ok, not an error
#define ERR_NULLARG         1   // NULL where it shouldn't be
#define ERR_INVARG          2   // invalid argument
#define ERR_MSG_ADD         3   // failed to add message
#define ERR_KEY_UPDT        4   // failed to update key
#define ERR_MSG_VRF         5   // message verification failed
#define ERR_MSG_SIZE        6   // wrong message size
#define ERR_SERIAL_RD       7   // failed to read message from serial port
#define ERR_BUFFSIZE        8   // buffer too small
#define ERR_TIMEOUT         9   // timeout

// requires rcvd_len, rcvd_hdr, rcvd_buff
// #define copy_rf12_to_buffer { rcvd_len = rf12_len; rcvd_hdr = rf12_hdr; memcpy(rcvd_buff, rf12_data, rf12_len); rf12_recvDone(); replyAck(); rf12_recvDone(); }
#define copy_rf12_to_buffer() { rcvd_len = rf12_len; rcvd_hdr = rf12_hdr; memcpy(rcvd_buff, rf12_data, rf12_len); replyAck(); rf12_recvDone(); }


#ifndef  __linux__

// sends acknowledgement if required
void replyAck();

// modes: MODE_SRC, MODE_DST
uint8_t createHeader(uint8_t id, uint8_t mode, bool requireACK);

void printBuffer(const uint8_t *buffer, const uint8_t len);

// substitutes (length+1)-th character with 0 and prints as string
// message buffer must be at least (length+1) uint8_ts long
void printMessage(char* message, const int length);

// prints packet header
void printHeader(uint8_t header);

// prints both header and message
void printPacket(uint8_t packet_hdr, uint8_t packet_len, uint8_t *packet_data);

// waits until (device is running for) 'end' milliseconds
// or receives packet - whichever is sooner
// returns true if packet was received or false otherwise
bool waitReceive(unsigned long end);

// if sending doesn't take enough, waits for the rest of required time
void sendWait(char* message, uint8_t length, uint8_t header, unsigned int end);

void printError(int err_num);

#else

uint64_t millis();

#endif // __linux__

#endif // _COMMON_H_
