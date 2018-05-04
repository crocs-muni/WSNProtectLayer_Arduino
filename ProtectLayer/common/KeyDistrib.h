/**
 * 	Original file (KeyDistrib.nc) from WSNProtectLayer (https://github.com/crocs-muni/WSNProtectLayer):
 *  
 *  Interface for functions related to key distribution.
 *  This interface specifies functions available in split-phase manner related to key distribution. New key discovery to direct neigbor can be initiated and key to base station or other node can be obtained.
 * 	@version   1.0
 * 	@date      2012-2014
 */

/**
 * Modified as part of the master thesis:
 * 
 * @file 	KeyDistrib.h
 * @author 	Martin Sarkany
 * @date 	05/2018
 */

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
	PL_key_t				m_key;							// current key
	std::vector<Node> 		m_nodes;						// vector of structures describing nodes
	uint8_t 				m_key_size;						// key size				
	uint8_t 				m_nodes_num;					// number of nodes
	uint32_t 				m_counters[MAX_NODE_NUM + 1];	// counters for each node's key
public:
	/**
	 * @brief Constructot
	 * 
	 * @param filename Name of the file with keys and IDs
	 */
	KeyDistrib(std::string &filename);

	/**
	 * @brief Get key shared with a node
	 * 
	 * @param nodeID 	Node ID
	 * @param pNodeKey 	Pointer to pointer to a key
	 * @return uint8_t 	SUCCESS or FAIL
	 */
	uint8_t getKeyToNodeB(uint8_t nodeID, PL_key_t** pNodeKey);
	
	/**
	 * @brief Get the hash key
	 * 
	 * @param pHashKey 	Pointer to pointer to hash key
	 * @return uint8_t 	SUCCESS or FAIL
	 */
	uint8_t getHashKeyB(PL_key_t** pHashKey);

	/**
	 * @brief Get key to BS. Always returns FAIL
	 * 
	 * @param pBSKey 	Whatever
	 * @return uint8_t 	FAIL
	 */
	uint8_t getKeyToBSB(PL_key_t** pBSKey);	
	
	
};

#endif // __linux__

#endif //  KEYDISTRIB_H