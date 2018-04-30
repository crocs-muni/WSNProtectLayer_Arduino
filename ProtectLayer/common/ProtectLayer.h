#ifndef PROTECTLAYER_H
#define PROTECTLAYER_H

#include "ProtectLayerGlobals.h"
#include "Crypto.h"
#include "KeyDistrib.h"
#include "CTP.h"

#define ENABLE_UTESLA   1   // TODO move to Makefile

// #undef __linux__ // TODO!!! REMOVE - just for VS Code syntax highliting

#ifdef __linux__

#include "uTESLAMaster.h"
#include <string>

#else 

#include "uTESLAClient.h"

#endif

class ProtectLayer {
    uint8_t         m_node_id;  // TODO not set yet
    uint8_t         m_msg_buffer[MAX_MSG_SIZE];

    AES             m_aes;
    AEShash         m_hash;
    AESMAC          m_mac;
    Crypto          m_crypto;
    KeyDistrib      m_keydistrib;

    CTP             m_ctp;

#ifdef __linux__
    uTeslaMaster    *m_utesla;
    int             m_slave_fd;
#else
#ifdef ENABLE_UTESLA
    uTeslaClient    m_utesla;
#endif
#endif

public:
#ifdef __linux__
    ProtectLayer(std::string slave_path, std::string &key_file);    // throws runtime_error if there is a problem with key file
    virtual ~ProtectLayer();

    uint8_t broadcastMessage(uint8_t *buffer, uint8_t size);    // TODO not implemented yet
    uint8_t sendTo(msg_type_t msg_type, uint8_t receiver, uint8_t *buffer, uint8_t size); // TODO not implemented yet
#else
    ProtectLayer();
    uint8_t sendCTP(msg_type_t msg_type, uint8_t *buffer, uint8_t size);    // TODO not implemented yet
    uint8_t sendTo(msg_type_t msg_type, uint8_t receiver, uint8_t *buffer, uint8_t size);   // TODO not implemented yet
    uint8_t sendToBS(msg_type_t msg_type, uint8_t *buffer, uint8_t size);   // TODO not implemented yet
    uint8_t receive(uint8_t *buffer, uint8_t buff_size, uint8_t *received_size, uint16_t timeout);
#endif

    uint8_t startCTP();

    uint8_t receive(uint8_t *buffer, uint8_t buff_size, uint8_t *received_size);
};

#endif //  PROTECTLAYER_H