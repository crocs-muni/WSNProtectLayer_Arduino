#ifndef PROTECTLAYER_H
#define PROTECTLAYER_H

#include "Crypto.h"
#include "KeyDistrib.h"

#ifdef __linux__
#include <string>
#endif

class ProtectLayer {
    AES         m_aes;
    AEShash     m_hash;
    AESMAC      m_mac;
    Crypto      m_crypto;
    KeyDistrib  m_keydistrib;
public:
#ifdef __linux__
    ProtectLayer(std::string &key_file);    // throws runtime_error if there is a problem with key file
#else
    ProtectLayer();
#endif
};

#endif //  PROTECTLAYER_H