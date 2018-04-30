#include <Arduino.h>
#include <EEPROM.h>
#include <RF12.h>
#include <avr/eeprom.h>

#include "AES.h"
#include "ProtectLayer.h"
#include "common.h"

// #define RADIO_FREQ  RF12_868MHZ
// #define RADIO_GROUP 10
// #define BAUD_RATE   115200

// #define MSG_STR     "16Blongstestmsg"
#define MSG_STR     "testmsg"
#define BUFFER_SIZE 66
#define NODES_NUM   4

uint8_t node_id = 1;
uint8_t recipient = 0;
uint8_t msg_buffer[BUFFER_SIZE];

ProtectLayer protect_layer;

void setup()
{
    Serial.begin(BAUD_RATE);
    // Serial.println("\n===== BS slave =====\n");
    // Serial.flush();

    // header = createHeader(node_id, 0, 0);

    // rf12_initialize(node_id, RADIO_FREQ, RADIO_GROUP);

    node_id = eeprom_read_byte(0);
    recipient = ((node_id - 1)  % NODES_NUM) + 2;

    Serial.print(node_id);
    Serial.print("->");
    Serial.println(recipient);

    randomSeed(analogRead(0) * node_id);
}

void loop()
{
    strcpy((char*) msg_buffer, MSG_STR);
    
    uint32_t start = millis();
    
    // start = millis() - start;
    // Serial.print("SND");
    // Serial.println(start);

    uint8_t rcvd_len = 0;
    uint8_t rval;
    while(millis() - start < (uint32_t) random(20) * 1000){
        if((rval = protect_layer.receive(msg_buffer, BUFFER_SIZE, &rcvd_len, 300)) == SUCCESS){
            Serial.print(node_id);
            Serial.println(" received:");
            printBuffer(msg_buffer, rcvd_len);
            msg_buffer[rcvd_len - 16] = 0;
            Serial.println((char*)msg_buffer + 3);  // should print "testtesttesttes"
            // break;
        }
    }

    if(!random(NODES_NUM)){
        if(protect_layer.sendTo(MSG_APP, recipient, msg_buffer, strlen(MSG_STR) + 1) != SUCCESS){
            Serial.println("Failed to send msg");
        }
    }

    // if(rval != SUCCESS){
    //     Serial.print(node_id);
    //     Serial.println(" failed to receive anything");
    // }
    
}
