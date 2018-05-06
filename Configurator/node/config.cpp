/**
 * @brief JeeLink app to save configuration
 * 
 * @file    config.cpp
 * @author  Martin Sarkany
 * @date    05/2018
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>

#include "conf_common.h"

#define BUFFER_SIZE 64

#define reply(response)     \
    Serial.write(response); \
    Serial.flush();

#define reply_ok()  reply(REPLY_OK)


uint8_t address    = 0;
uint8_t value      = 0;
String  line;


void saveNodeID(uint8_t node_id)
{
    eeprom_update_byte(0, node_id);
}

void saveNodeKey(uint8_t *key, uint8_t node_id)
{
    eeprom_update_block(key, CONFIG_START_ADDRESS + (node_id * KEY_SIZE), KEY_SIZE);
}

#define saveBSKey(key)saveNodeKey(key, 0)

void saveuTESLAKey(uint8_t *key)
{
    eeprom_update_block(key, UTESLA_KEY_ADDRESS, KEY_SIZE);
}

void saveNeighbors(uint8_t *neighbors)
{
    eeprom_update_block(neighbors, NEIGHBORS_ADDRESS, 4);
}

void readNodeKey(uint8_t *key, uint8_t node_id)
{
    eeprom_read_block(key, CONFIG_START_ADDRESS + (node_id * KEY_SIZE), KEY_SIZE);
}

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    uint8_t buffer[BUFFER_SIZE];
    if(Serial.available() > 0){
        uint8_t len1 = Serial.read();
        while(Serial.available() < 1);
        uint8_t len2 = Serial.read();
        if(len1 != len2){
            reply(REPLY_ERR_LEN);
            return;
        }

        while(Serial.available() < 1);
        if(Serial.readBytes(reinterpret_cast<char*>(buffer), len1) != len1){
            reply(REPLY_ERR_MSG_SIZE);
            return;
        }

        if(buffer[0] == CFG_ID){
            if(len1 < 2){
                reply(REPLY_ERR_MSG_SIZE); // TODO maybe different error code
                return;
            }
            saveNodeID(buffer[1]);
            reply_ok();
        } else if(buffer[0] == CFG_BS_KEY){
            if(len1 < KEY_SIZE + 1){
                reply(REPLY_ERR_MSG_SIZE); // TODO maybe different error code
                return;
            }
            saveBSKey(buffer + 1);
            reply_ok();
        } else if(buffer[0] == CFG_NODE_KEY){
            if(len1 < KEY_SIZE + 2){
                reply(REPLY_ERR_MSG_SIZE); // TODO maybe different error code
                return;
            }
            saveNodeKey(buffer + 2, buffer[1]);
            reply_ok();
        } else if(buffer[0] == CFG_REQ_KEY){
            if(len1 < 2){
                reply(REPLY_ERR_MSG_SIZE); // TODO maybe different error code
                return;
            }
            readNodeKey(buffer, buffer[1]);
            Serial.write(buffer, KEY_SIZE);
        } else if(buffer[0] == CFG_UTESLA_KEY){
            if(len1 < KEY_SIZE + 1){
                reply(REPLY_ERR_MSG_SIZE); // TODO maybe different error code
                return;
            }
            saveuTESLAKey(buffer + 1);
            reply_ok();
        } else if(buffer[0] == CFG_NEIGHBORS){
            if(len1 < 4){
                reply(REPLY_ERR_MSG_SIZE);
                return;
            }
            saveNeighbors(buffer + 1);
            reply_ok();
        } else {
            reply(REPLY_ERR_MSG_TYPE);
            return;
        }

        // TODO finalizing message => sleep

        // // write only if it changes (limit writes)
        // if(EEPROM.read(address) != value){
        //     EEPROM.write(address, value);
        // }

        // // check stored value
        // if(EEPROM.read(address) == value){
        //     // Serial.write("OK");
        //     Serial.flush();
        // } else {
        //     Serial.write("Failed to write to EEPROM");
        //     Serial.flush();
        //     set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        //     sleep_enable();
        //     sleep_mode();
        // }

    }
}
