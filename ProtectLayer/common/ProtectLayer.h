#ifndef PROTECTLAYER_H
#define PROTECTLAYER_H

#include "Crypto.h"
#include "KeyDistrib.h"

#ifdef __linux__
#include "uTESLAMaster.h"
#include <string>
#endif

// #define ENABLE_UTESLA

class ProtectLayer {
    AES             m_aes;
    AEShash         m_hash;
    AESMAC          m_mac;
    Crypto          m_crypto;
    KeyDistrib      m_keydistrib;

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
#else
    ProtectLayer();
#endif
};

#endif //  PROTECTLAYER_H