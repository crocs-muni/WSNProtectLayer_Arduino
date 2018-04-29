#include "uTESLAClient.h"
#include <EEPROM.h>
#include <avr/sleep.h>


#include "common.h"
#include "AES.h"
#include "AES_crypto.h"
#include "KeyDistrib.h"
#include "ProtectLayer.h"

#define MESSAGE_BUFF_NUM    3      // buffer for 10 messages
#define CLIENT_DURATION     20000   // 10 seconds


// typedef struct {
//     int8_t round_num;
//     uint8_t len;
//     uint8_t buffer[70];
// } message_t;

// // uint8_t         last_hash[] = { 0x2D, 0x2F, 0x62, 0xFC, 0x1B, 0x06, 0xB9, 0x33, 0x5E, 0x4A, 0x22, 0x0D, 0x2B, 0x60, 0xBC, 0x5B };
// uint8_t         last_hash[] = { 0xA4, 0x7E, 0xB0, 0x22, 0x52, 0xDA, 0xE2, 0xAA, 0x0B, 0xCD, 0x2D, 0xAE, 0xEF, 0xFE, 0xFE, 0xBD };

// message_t       messages[MESSAGE_BUFF_NUM];

// uint8_t         node_id = 1;

// uint8_t         rcvd_len;
// uint8_t         rcvd_hdr;
// uint8_t         rcvd_buff[80];

// // BlakeHMAC       hmac;
// // Blake224        hash;
// AES             aes;
// AESMAC          hmac(&aes);
// AEShash         hash(&aes);
// uTeslaClient    utesla_client(last_hash, &hash, &hmac);

// static void initMessages()
// {
//     memset(messages, -1, MESSAGE_BUFF_NUM * sizeof(message_t));
// }

// static bool addMessage(const uint8_t *message, const uint8_t message_size)
// {
//     for(int i=0;i<=MESSAGE_BUFF_NUM;i++){
//         if(messages[i].round_num <= utesla_client.getRoundNum()){
//             messages[i].round_num = utesla_client.getRoundNum() + 1;
//             messages[i].len = message_size;
//             memcpy(messages[i].buffer, message, message_size);
//             return true;
//         }
//     }

//     return false;
// }

// static void verifyMessages()
// {
//     for(int i=0;i<MESSAGE_BUFF_NUM;i++){
//         // Serial.println(i, DEC);
//         if(messages[i].round_num != utesla_client.getRoundNum()){
//             continue;
//         }
//         // Serial.println(messages[i].round_num, DEC);
//         uint8_t message_len = messages[i].len - utesla_client.getMacSize();
//         Serial.print("Msg ");
//         printBuffer(messages[i].buffer, message_len);
//         if(utesla_client.verifyMAC(messages[i].buffer, message_len, messages[i].buffer + message_len)){
//             Serial.println("OK");
//             Serial.flush();
//         } else {
//             Serial.println("NOK");
//             Serial.flush();
//         }
//     }
// }

// void setup()
// {
//     Serial.begin(BAUD_RATE);
//     Serial.println("\nuTESLA client\n");
//     Serial.flush();

//     rf12_initialize(node_id, RADIO_FREQ, RADIO_GROUP);
//     initMessages();
// }

// void loop()
// {
//     uint32_t loop_start = millis();

//     while(waitReceive(loop_start + CLIENT_DURATION)){
//         copy_rf12_to_buffer;
//         Serial.print("Rcvd: ");
//         printBuffer(rcvd_buff, rcvd_len);
//         if(rcvd_buff[0] == MSG_TYPE_DATA){
//             // Serial.print("Msg add ");
//             if(!addMessage(rcvd_buff + 1, rcvd_len - 1)){
//                 printError(ERR_MSG_ADD);
//             }
//             Serial.flush();
//         } else if(rcvd_buff[0] == MSG_TYPE_KEY){
//             // check message length
//             if(rcvd_len - 1 < utesla_client.getKeySize()){
//                 // Serial.println(err_msg);
//                 // Serial.flush();
//                 printError(ERR_KEY_UPDT);
//                 continue;
//             }
//             // update key
//             // Serial.print("Key update ");
//             if(!utesla_client.updateKey(rcvd_buff + 1)){
//                 printError(ERR_KEY_UPDT);
//             }
//             Serial.flush();
//             // verify messages from previous round
//             verifyMessages();
//             // Serial.println("done vrfng");
//             // Serial.flush();
//         } else {
//             // Serial.println(err_msg);
//             // Serial.flush();
//             printError(ERR_INVARG);
//         }
//     }

//     Serial.println("Finished");
//     Serial.flush();
//     set_sleep_mode(SLEEP_MODE_PWR_DOWN);
//     sleep_enable(); 
//     sleep_mode();
// }


// AES aes;
// AESMAC mac(&aes);
// AEShash hash(&aes);
// KeyDistrib keydistrib;
ProtectLayer protect_layer;

void setup()
{
}

void loop()
{

}