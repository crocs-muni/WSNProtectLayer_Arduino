#ifndef PROTECTLAYERGLOBALS_H
#define PROTECTLAYERGLOBALS_H

#include <stdint.h>

#define ENABLE_UTESLA           1   // TODO move to Makefile

#define MAX_MSG_SIZE            66  // maximum message length for RF12 radio

#define CTP_DURATION_MS         10000
#define CTP_REBROADCASTS_NUM    5
#define CTP_REBROADCASTS_DELAY	500

#define DEFAULT_REQ_ACK         0

#define NODE_RECV_TIMEOUT_MS    100

#define BS_NODE_ID              1

// #define AUTOFORWARD				1	// auto forwarding of messages for BS

// TODO! enum or defines for all return values
#define FORWARD					5


typedef uint8_t node_id_t;
typedef uint8_t msg_type_t;

/**
	The enumeration of possible message type
*/
typedef enum _MSG_TYPE {
	MSG_OTHER = 0,	/**< message of other (unknown) type */
	MSG_APP,	/**< application message */
	MSG_ROUTE,	/**< message of routing component */
	MSG_FORWARD, /**< message to be forwarded to BS */
    MSG_CTP,    /**< CTP distance message */
	MSG_BRDCST,	/**< uTESLA broadcast message */
	MSG_COUNT   /**< number of message types */
} MSG_TYPE;


#pragma pack(push, 1)
/**
	A structure representing security header
*/
typedef struct SPHeader {
    msg_type_t  msgType;	/**< type of message */
//     uint8_t privacyLevelIndicator;	/**< MSb is used as a flag for Phantom routing, other bits indicate privacy level applied */
    uint8_t     sender;	/**< sender ID */
    uint8_t     receiver; /**< receiver ID */
// #ifdef PLAINTEXT_DEMO
//     uint8_t plaintext[PLAINTEXT_BYTES];
// #endif
} SPHeader_t;

#pragma pack(pop)

#define SPHEADER_SIZE	sizeof(SPHeader_t)

#endif //  PROTECTLAYERGLOBALS_H