#ifndef PROTECTLAYER_H
#define PROTECTLAYER_H

#include "Crypto.h"
#include "KeyDistrib.h"

class ProtectLayer {
    AES         m_aes;
    AEShash     m_hash;
    AESMAC      m_mac;
    Crypto      m_crypto;
    KeyDistrib  m_keydistrib;
public:
    ProtectLayer();
};

#endif //  PROTECTLAYER_H