#ifndef COMMON_H
#define COMMON_H


// TODO! use correct size from the rest of the library
#ifndef KEY_SIZE
#define KEY_SIZE 16
#endif

#define NEIGHBORS_ADDRESS       (uint8_t*)0x3C
#define CONFIG_START_ADDRESS    (uint8_t*)0x40  // TODO rename (KEY_START_ADDRESS?)
#define UTESLA_KEY_ADDRESS      (uint8_t*)0x1F4


#define CFG_TYPE_SIZE   1

#define CFG_ID          0x58
#define CFG_BS_KEY      1
#define CFG_NODE_KEY    2
#define CFG_UTESLA_KEY  3
#define CFG_REQ_KEY     4
#define CFG_NEIGHBORS   5

#define REPLY_OK                0
#define REPLY_DONE              1
#define REPLY_ERR_LEN           2
#define REPLY_ERR_DONE          3
#define REPLY_ERR_MSG_SIZE      4
#define REPLY_ERR_EEPROM        5
#define REPLY_ERR_MSG_TYPE      6

#endif //  COMMON_H