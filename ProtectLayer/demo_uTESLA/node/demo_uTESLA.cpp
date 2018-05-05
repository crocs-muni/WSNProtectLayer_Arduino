#include <Arduino.h>
#include <EEPROM.h>
#include <RF12.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>

#include "AES.h"
#include "ProtectLayer.h"
#include "common.h"


#define BUFFER_SIZE 40
#define NODES_NUM   4

uint8_t node_id     = 2;        // TODO use ProtectLayer::getNodeID()
uint8_t rcvd_buffer[BUFFER_SIZE];
uint8_t uTESLA_buffer[BUFFER_SIZE];
uint8_t uTESLA_size = 0;

ProtectLayer protect_layer;

void setup()
{
    Serial.begin(BAUD_RATE);

    node_id = protect_layer.getNodeID();
    protect_layer.discoverNeighbors();

    uint32_t neighbors = protect_layer.getNeighbors();

    Serial.println("Neighbors:");
    for(int i=0; i < 32; i++){
        if(bitIsSet(neighbors, i)){
            Serial.print(i);
            Serial.print(" ");
        }
    }
    Serial.println();
}

void loop()
{
    uint8_t rcvd_len = 0;
    uint8_t rval;
    SPHeader_t *spheader = reinterpret_cast<SPHeader_t*>(rcvd_buffer);

    if((rval = protect_layer.receive(rcvd_buffer, BUFFER_SIZE, &rcvd_len, 300)) == SUCCESS){
        Serial.print(node_id);
        Serial.println(" rcvd:");
        printBuffer(rcvd_buffer, rcvd_len);

        if(spheader->msgType == MSG_UTESLA){
            memcpy(uTESLA_buffer, rcvd_buffer, rcvd_len);
            uTESLA_size = rcvd_len;
        }

        // break;
    } else if(rval == FORWARD){
        Serial.println("fwd");
        if(spheader->msgType == MSG_UTESLA_KEY){
            if(protect_layer.verifyMessage(uTESLA_buffer, uTESLA_size) == SUCCESS){
                Serial.println("msg ok");
            } else {
                Serial.println("msg nok");
            }
        }
    }


}
