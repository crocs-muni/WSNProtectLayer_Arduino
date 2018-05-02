#include <Arduino.h>
#include <EEPROM.h>
#include <RF12.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>

#include "AES.h"
#include "ProtectLayer.h"
#include "common.h"


// #define MSG_STR     "16Blongstestmsg"
// #define MSG_STR     "testmsg"
#define BUFFER_SIZE 40
#define NODES_NUM   4

uint8_t node_id     = 2;        // TODO use ProtectLayer::getNodeID()
uint8_t msg_buffer[BUFFER_SIZE];
// uint8_t own_msg_buffer[BUFFER_SIZE];

uint8_t ctp_result  = FAIL;

ProtectLayer protect_layer;

void setup()
{
    Serial.begin(BAUD_RATE);

    node_id = eeprom_read_byte(0);

    // randomSeed(analogRead(0) * node_id);

    if((ctp_result = protect_layer.startCTP()) != SUCCESS){
        Serial.println("CTP failed");
        Serial.flush();
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable(); 
        sleep_mode(); 
    }
}

void loop()
{
    uint32_t start = millis();

    if(ctp_result != SUCCESS){ // sometimes it wakes up
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable(); 
        sleep_mode(); 
    }

    uint8_t rcvd_len = 0;
    uint8_t rval;
    while(millis() - start < (uint32_t) random(20) * 1000){
        if((rval = protect_layer.receive(msg_buffer, BUFFER_SIZE, &rcvd_len, 300)) == SUCCESS){
            Serial.print(node_id);
            Serial.println(" rcvd:");
            printBuffer(msg_buffer, rcvd_len);
            msg_buffer[rcvd_len - 16] = 0;
            Serial.println((char*)msg_buffer + 3);
            // break;
        } else {
            if(rval == FORWARD){
                Serial.println("fwd");
            }
        }
    }

    if(!random(7)){
        Serial.print("snd: ");
        // Serial.println(node_id);
        memset(msg_buffer, node_id , BUFFER_SIZE);
        // msg_buffer[0] = node_id;
        if(protect_layer.sendToBS(MSG_FORWARD, msg_buffer, 4) != SUCCESS){
            Serial.println("Fail");
        }
    }

    // set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    // sleep_enable(); 
    // sleep_mode(); 
    // strcpy((char*) msg_buffer, MSG_STR);
    
    // uint32_t start = millis();

    // uint8_t rcvd_len = 0;
    // uint8_t rval;
    // while(millis() - start < (uint32_t) random(20) * 1000){
    //     if((rval = protect_layer.receive(msg_buffer, BUFFER_SIZE, &rcvd_len, 300)) == SUCCESS){
    //         Serial.print(node_id);
    //         Serial.println(" received:");
    //         printBuffer(msg_buffer, rcvd_len);
    //         msg_buffer[rcvd_len - 16] = 0;
    //         Serial.println((char*)msg_buffer + 3);  // should print "testtesttesttes"
    //         // break;
    //     }
    // }

    // if(!random(NODES_NUM)){
    //     if(protect_layer.sendTo(MSG_APP, recipient, msg_buffer, strlen(MSG_STR) + 1) != SUCCESS){
    //         Serial.println("Failed to send msg");
    //     }
    // }

}
