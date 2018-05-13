/**
 * @brief Implementation of a slave device for Linux base station
 * 
 * @file    BS_slave.cpp
 * @author  Martin Sarkany
 * @date    05/2018
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <RF12.h>

#include "common.h"
#include "ProtectLayerGlobals.h"

#define BUFFER_SIZE 80

uint8_t node_id = BS_NODE_ID;
uint8_t header = 0;

uint8_t rcvd_len;
uint8_t rcvd_hdr;
uint8_t rcvd_buff[MAX_MSG_SIZE];

void setup()
{
    Serial.begin(BAUD_RATE);

    header = createHeader(node_id, MODE_SRC, 0);

    rf12_initialize(node_id, RADIO_FREQ, RADIO_GROUP);
}


// TODO! "unicast" - only broadcast now
void loop()
{
    char buffer[BUFFER_SIZE];
    if(Serial.available() > 0){
        // read 1st length byte
        uint8_t len1 = Serial.read();

        // read 2nd length byte
        while(Serial.available() < 1);
        uint8_t len2 = Serial.read();

        // compare length bytes
        if(len1 != len2){
            printError(ERR_MSG_SIZE);
            return;
        }

        // read data
        while(Serial.available() < 1);
        if(Serial.readBytes(buffer, len1) != len1){
            printError(ERR_SERIAL_RD);
            return;
        }

        // send
        rf12_sendNow(header, buffer, len1);

        // ack
        printError(ERR_OK);
    }

    // receive from radio
    if(rf12_recvDone() && !rf12_crc){
        if((rf12_hdr & RF12_HDR_MASK) != BS_NODE_ID){
            rf12_recvDone();
            return;
        }

        if(rf12_data[0] == MSG_CTP){
            return;
        }

        copy_rf12_to_buffer();

        // send to host
        Serial.write(rcvd_len);
        Serial.write(rcvd_buff, rcvd_len);
        Serial.flush();
    }
}