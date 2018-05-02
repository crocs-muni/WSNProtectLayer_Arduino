#include <Arduino.h>
#include <EEPROM.h>
#include <RF12.h>

#include "common.h"
#include "ProtectLayerGlobals.h"

// #define RADIO_FREQ  RF12_868MHZ
// #define RADIO_GROUP 10
// #define BAUD_RATE   115200

#define BUFFER_SIZE 80

uint8_t node_id = BS_NODE_ID;
uint8_t header = 0;


void setup()
{
    Serial.begin(BAUD_RATE);
    // Serial.println("\n===== BS slave =====\n");
    // Serial.flush();

    header = createHeader(node_id, MODE_SRC, 0);

    rf12_initialize(node_id, RADIO_FREQ, RADIO_GROUP);
}


// TODO! "unicast" - only broadcast now
void loop()
{
    char buffer[BUFFER_SIZE];
    if(Serial.available() > 0){
        uint8_t len1 = Serial.read();
        while(Serial.available() < 1);
        uint8_t len2 = Serial.read();
            // Serial.write(len1);
            // Serial.write(len2);
        if(len1 != len2){
            // Serial.print("Incorrect length byte: ");
            // Serial.println(len2);
            printError(ERR_MSG_SIZE);
            // Serial.flush();
            return;
        }

        while(Serial.available() < 1);
        if(Serial.readBytes(buffer, len1) != len1){
            // Serial.println("Failed to receive whole packet from serial port");
            printError(ERR_SERIAL_RD);
            // Serial.flush();
            return;
        }

        // Serial.println("Sending: ");
        // printBuffer(buffer, len1);
        // Serial.flush();

        rf12_sendNow(header, buffer, len1);

        printError(ERR_OK);
    }

    if(rf12_recvDone() && !rf12_crc){
        if((rf12_hdr & RF12_HDR_MASK) != BS_NODE_ID){
            rf12_recvDone();
            return;
        }

        if(buffer[0] == MSG_CTP){
            return;
        }

        // message_len = rf12_len + RF12_HDR_SIZE;
        uint8_t rcvd_len;
        uint8_t rcvd_hdr;
        uint8_t rcvd_buff[MAX_MSG_SIZE];
        // memcpy(message_buffer, rf12_buf, message_len);
        // memcpy(message_buffer, rf12_data, message_len);
        // replyAck();
        // rf12_recvDone();
        copy_rf12_to_buffer();
        Serial.write(rcvd_buff, rcvd_len);
    }
}