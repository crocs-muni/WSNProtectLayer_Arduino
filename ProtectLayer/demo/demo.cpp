/**
 * @brief Demo app showing one-hop communication
 * 
 * @file    demo.cpp
 * @author  Martin Sarkany
 * @date    05/2018
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <RF12.h>
#include <avr/eeprom.h>

#include "AES.h"
#include "ProtectLayer.h"
#include "common.h"


// #define MSG_STR     "16Blongstestmsg"
#define MSG_STR     "testmsg"
#define BUFFER_SIZE 66
#define NODES_NUM   6

uint8_t node_id = 1;
uint8_t recipient = 0;
uint8_t msg_buffer[BUFFER_SIZE];

ProtectLayer protect_layer;

void setup()
{
    Serial.begin(BAUD_RATE);

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

    uint8_t rcvd_len = 0;
    uint8_t rval;
    while(millis() - start < (uint32_t) random(20) * 1000){
        if((rval = protect_layer.receive(msg_buffer, BUFFER_SIZE, &rcvd_len, 300)) == SUCCESS){
            Serial.print(node_id);
            Serial.println(" received:");
            printBuffer(msg_buffer, rcvd_len);
            msg_buffer[rcvd_len - 16] = 0;
            Serial.println((char*)msg_buffer + 3);
        }
    }

    if(!random(NODES_NUM)){
        if(protect_layer.sendTo(MSG_APP, recipient, msg_buffer, strlen(MSG_STR) + 1) != SUCCESS){
            Serial.println("Failed to send msg");
        }
    }
}
