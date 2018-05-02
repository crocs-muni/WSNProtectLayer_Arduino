#ifndef KEYDISTRIB_H
#define KEYDISTRIB_H

#include <stdint.h>

#include "common.h"
#include "AES.h"    // PL_key_t defined there for now

#ifndef __linux__

class KeyDistrib {
private:
	PL_key_t m_key;
	uint32_t m_counters[MAX_NODE_NUM + 1];
public:
	KeyDistrib(); // TODO! remove
	// /**
	// 	Command: Posts taks for key task_discoverKeys for key discovery
	// 	@return error_t status. SUCCESS or EALREADY if already pending
	// */
	// uint8_t discoverKeys();
	
	
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

	uint8_t deleteKey(uint8_t nodeID); // TODO
};

#else // __linux__
#include "configurator.h"
// version for linux base station
#include <vector>
#include <string>

class KeyDistrib {
private:
	PL_key_t				m_key;
	std::vector<Node> 		m_nodes;
	uint8_t 				m_key_size;
	uint8_t 				m_nodes_num;
	uint32_t 				m_counters[MAX_NODE_NUM + 1];
public:
	KeyDistrib(std::string &filename);


	uint8_t getKeyToNodeB(uint8_t nodeID, PL_key_t** pNodeKey);
	
	uint8_t getKeyToBSB(PL_key_t** pBSKey);	
	
	uint8_t getHashKeyB(PL_key_t** pHashKey);

	uint8_t deleteKey(uint8_t nodeID); // TODO
};

#endif // __linux__

#endif //  KEYDISTRIB_H