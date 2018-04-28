#include "ProtectLayer.h"

#ifdef __linux__
ProtectLayer::ProtectLayer(std::string &key_file):m_hash(&m_aes), m_mac(&m_aes), m_keydistrib(key_file), m_crypto(&m_aes, &m_mac, &m_hash, &m_keydistrib)
#else
ProtectLayer::ProtectLayer():m_hash(&m_aes), m_mac(&m_aes), m_crypto(&m_aes, &m_mac, &m_hash, &m_keydistrib)
#endif
{
    
}