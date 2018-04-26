#include "KeyDistrib.h"

#include <string.h>


KeyDistrib::KeyDistrib()
{
    memset((void*) &m_key, 0, sizeof(PL_key_t));
}

uint8_t KeyDistrib::discoverKeys()
{
    // TODO!
}


uint8_t KeyDistrib::getKeyToNodeB(uint8_t nodeID, PL_key_t** pNodeKey)
{
    // TODO!
    *pNodeKey = &m_key;
}

uint8_t KeyDistrib::getKeyToBSB(PL_key_t** pBSKey)
{
    // TODO!
    *pBSKey = &m_key;
}

uint8_t KeyDistrib::getHashKeyB(PL_key_t** pHashKey)
{
    // TODO!
    *pHashKey = &m_key;
}
