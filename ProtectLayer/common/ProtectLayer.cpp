#include "ProtectLayer.h"

ProtectLayer::ProtectLayer():m_hash(&m_aes), m_mac(&m_aes), m_crypto(&m_aes, &m_mac, &m_hash, &m_keydistrib)
{
    
}