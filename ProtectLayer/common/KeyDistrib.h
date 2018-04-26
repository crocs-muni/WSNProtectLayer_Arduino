#ifndef KEYDISTRIB_H
#define KEYDISTRIB_H

#include <stdint.h>

#include "AES.h"    // PL_key_t defined there for now

class KeyDistrib {
public:
	/**
		Command: Posts taks for key task_discoverKeys for key discovery
		@return error_t status. SUCCESS or EALREADY if already pending
	*/
	uint8_t discoverKeys();
	
	
	/**
		Command: Get key to node.
		@param[in] nodeID node identification of node for which the key should be searched for
		@param[out] pNodeKey handle to key shared between node and base station 
		@return error_t status.
	*/	
	uint8_t getKeyToNodeB(uint8_t nodeID, PL_key_t** pNodeKey);
	
	/**
		Command: Get key to base station
		@param[out] pBSKey handle to key shared between node and base station 
		@return error_t status.
	*/
	uint8_t getKeyToBSB(PL_key_t** pBSKey);	
	
	
	/**
		Command: Get key for AES based hashing function 
		@param[out] pBSKey handle to key
		@return error_t status.
	*/
	uint8_t getHashKeyB(PL_key_t** pHashKey);
};


#endif //  KEYDISTRIB_H