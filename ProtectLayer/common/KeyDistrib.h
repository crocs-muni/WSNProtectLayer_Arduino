/**
 * 	Original file (KeyDistrib.nc) from WSNProtectLayer (https://github.com/crocs-muni/WSNProtectLayer):
 *  
 *  Interface for functions related to key distribution.
 *  This interface specifies functions available in split-phase manner related to key distribution. New key discovery to direct neigbor can be initiated and key to base station or other node can be obtained.
 * 	@version   1.0
 * 	@date      2012-2014
 * 
 * 
 * Copyright (c) 2010-2014, Masaryk University
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
	PL_key_t m_key;							// key structure holding current key
	uint32_t m_nodes_list;					// list (eat bit represents a neighbor) of all nodes in the network
	uint32_t *m_neighbors;					// pointer to list all available neighbors
	uint32_t m_counters[MAX_NODE_NUM + 1];	// counters for every keys // TODO remove counter for key 0 - does not exist
public:

	/**
	 * @brief Constructor, initializes attributes
	 * 
	 */
	KeyDistrib(uint32_t *m_neighbors);

	/**
		Command: Get key to node.
		@param[in] nodeID node identification of node for which the key should be searched for
		@param[out] pNodeKey handle to key shared between node and base station 
		@return error_t status.
	*/	
	uint8_t getKeyToNodeB(uint8_t nodeID, PL_key_t** pNodeKey);

	/**
	 * @brief Get key that has been derived during handshake in neighbor discovery
	 * 
	 * @param nodeID 	Node's ID
	 * @param pNodeKey 	Pointer to a pointer to a requested key
	 * @return uint8_t 	SUCCESS or FAIL
	 */
	uint8_t getDerivedKeyToNodeB(uint8_t nodeID, PL_key_t** pNodeKey);
	
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

	/**
	 * @brief Overwrite key in EEPROM
	 * 
	 * @param nodeID 	Node's ID
	 * @return uint8_t 	SUCCESS or FAIL
	 */
	uint8_t deleteKey(uint8_t nodeID);

	uint8_t deriveKeyToNode(uint8_t nodeID, uint8_t *random_input, uint8_t random_input_size, MAC *mac);
	
	uint32_t getNodesList();
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