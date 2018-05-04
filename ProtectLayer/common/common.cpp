/**
 * @brief Definitions for functions common in whole project.
 * 
 * @file    common.cpp
 * @author  Martin Sarkany
 * @date    05/2018
 */

#include "common.h"

#ifndef __linux__

void replyAck()
{
    if(RF12_WANTS_ACK){
        rf12_sendStart(RF12_ACK_REPLY,0,0);
    }
}

uint8_t createHeader(uint8_t id, uint8_t mode, bool requireACK)
{
    uint8_t header = requireACK ? RF12_HDR_ACK : 0 ;
    header |= (mode ? RF12_HDR_DST : 0) | id;
    
    return header;
}

void printBuffer(const uint8_t *buffer, const uint8_t len)
{
    for(int i=0;i<len;i++){
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    Serial.flush();
}

bool waitReceive(uint32_t end)
{
    while(1){
        // return false if the time has passed
        if(millis() >= end){
            return false;
        }
        // return true if packet was received
        if(rf12_recvDone() && rf12_crc == 0){
            return true;
        }
    }

    return false;   // unreachable
}

void printError(int err_num)
{
    Serial.write(err_num);
    Serial.flush();
}

int freeRam() {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


#else
#include <iostream>
#include <sys/timeb.h>

void printDebug(const char*msg, bool is_error)
{
#ifdef DEBUG
    if(is_error){
        std::cerr << msg << std::endl;
    } else {
        std::cout << msg << std::endl;
    }
#endif
}

void printBufferHex(const uint8_t *buffer, const uint32_t len)
{
    for(uint32_t i=0;i<len;i++){
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}

uint64_t millis(){
    timeb tb;
    ftime(&tb);
    return tb.millitm + (tb.time * 1000);
}


#endif // __linux__
