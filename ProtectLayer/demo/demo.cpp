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

#define MSG_STR     "testtesttesttes"
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

    randomSeed(analogRead(0)  + node_id);
}

void loop()
{
    delay(random(1000));

    strcpy(msg_buffer, MSG_STR);
    if(protect_layer.sendTo(MSG_APP, recipient, msg_buffer, strlen(MSG_STR) + 1) != SUCCESS){
        Serial.println("Failed to send message");
    }

    uint8_t rcvd_len = 0;
    uint8_t rval;
    for(int i=0;i<5;i++){
        if((rval = protect_layer.receive(msg_buffer, BUFFER_SIZE, &rcvd_len)) == SUCCESS){
            Serial.println("Received:");
            printBuffer(msg_buffer, rcvd_len);
            break;
        }
    }

    if(rval != SUCCESS){
        Serial.println("Failed to receive any message");
    }
}