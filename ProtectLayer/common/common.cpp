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


// message buffer must be at least (length+1) uint8_ts long
void printMessage(char* message, const int length)
{
    message[length] = 0;
    Serial.println(message);
}

void printHeader(uint8_t header)
{
    if(header & RF12_HDR_DST){
        Serial.print("DST: ");
        Serial.println(header ^ RF12_HDR_DST);
    } else {
        Serial.print("SRC: ");
        Serial.println(header);
    }
}

void printPacket(uint8_t packet_hdr, uint8_t packet_len, uint8_t *packet_data)
{
    printHeader(packet_hdr);
    Serial.print("Length: ");
    Serial.println(packet_len, DEC);
    Serial.print("Message: ");
    printMessage(packet_data, packet_len);
    Serial.println();
}

bool waitReceive(uint32_t end)
{
    while(1){
        if(millis() >= end){
            return false;
        }
        if(rf12_recvDone() && rf12_crc == 0){
            return true;
        }
    }

    return false;   // unreachable
}

void sendWait(char* message, uint8_t length, uint8_t header, unsigned int end)
{
    Serial.print("Sending message: ");
    Serial.println(message);
    unsigned time_to_wait;
    rf12_sendNow(header, message, length);
    rf12_sendWait(0);
    if((time_to_wait = end - millis()) > 0){
        delay(time_to_wait);    
    }
}

void printError(int err_num)
{
    // Serial.print(ERROR_MESSAGE);
    // Serial.println(err_num, DEC);
    Serial.write(err_num);
    Serial.flush();
}

#else
#include <sys/timeb.h>

uint64_t millis(){
    timeb tb;
    ftime(&tb);
    return tb.millitm + (tb.time * 1000);
}


#endif // __linux__
