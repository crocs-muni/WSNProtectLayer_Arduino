#include <Arduino.h>
#include <EEPROM.h>
#include <RF12.h>

#include "common.h"

// #define RADIO_FREQ  RF12_868MHZ
// #define RADIO_GROUP 10
// #define BAUD_RATE   115200

#define BUFFER_SIZE 80

uint8_t node_id = 1;
uint8_t header = 0;


void setup()
{
    Serial.begin(BAUD_RATE);
    // Serial.println("\n===== BS slave =====\n");
    // Serial.flush();

    header = createHeader(node_id, 0, 0);

    rf12_initialize(node_id, RADIO_FREQ, RADIO_GROUP);
}

void loop()
{
    char buffer[BUFFER_SIZE];
    if(Serial.available() > 0){
        uint8_t len1 = Serial.read();
        while(Serial.available() < 1);
        uint8_t len2 = Serial.read();
        if(len1 != len2){
            Serial.print("Incorrect length byte: ");
            Serial.println(len2);
            Serial.flush();
            return;
        }

        while(Serial.available() < 1);
        if(Serial.readBytes(buffer, len1) != len1){
            Serial.println("Failed to receive whole packet from serial port");
            Serial.flush();
            return;
        }

        Serial.println("Sending: ");
        printBuffer(buffer, len1);
        Serial.flush();

        rf12_sendNow(header, buffer, len1);
    }
}