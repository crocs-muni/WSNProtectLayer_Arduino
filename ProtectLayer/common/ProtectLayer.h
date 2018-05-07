/**
 * @brief Header file for ProtectLayer
 * 
 * @file ProtectLayer.h
 * @author Martin Sarkany
 * @date 05/2018
 */

#ifndef PROTECTLAYER_H
#define PROTECTLAYER_H

#include "ProtectLayerGlobals.h"
#include "Crypto.h"
#include "KeyDistrib.h"
#include "CTP.h"

#undef __linux__ // TODO! REMOVE - just for VS Code syntax highlighting

#ifdef __linux__
#include <string>

#include "uTESLAMaster.h"
#include "configurator.h"
#else 
#include "uTESLAClient.h"
#endif

/**
 * @brief Class that provides security features for WSN communication over RF12 radio. Works on Linux host as base station and JeeLink devices as a regular node.
 * BS requires slave node to provide RF12 communication.
 * 
 */
class ProtectLayer {
private:
    AES             m_aes;          // single-block AES encryption
    AEShash         m_hash;         // AES-based hash computation, uses m_aes for encryption
    AESMAC          m_mac;          // AES-based MAC computation, uses m_aes for encryption
    KeyDistrib      m_keydistrib;   // provides keys for m_crypto
    Crypto          m_crypto;       // provides all crypto operations, uses m_aes, m_hash and m_mac

#ifdef ENABLE_CTP
    CTP             m_ctp;          // class providing CTP establishment, required when routing to BS
#endif // ENABLE_CTP

    uint8_t         m_received[2];  // one byte of last received MAC or key - not to rebroadcast already rebroadcasted message or key
                                    // TODO use sequence number or something

#ifdef __linux__
    uTeslaMaster    *m_utesla;      // uTESLA class for BS
    int             m_slave_fd;     // file descriptor of slave JeeLink device
#else
    uint8_t         m_node_id;      // this node's ID
    uint32_t        m_neighbors;    // active neighors, available only after neighbor discovery

    /**
     * @brief Forward uTESLA messages
     * 
     * @param buffer    Message
     * @param size      Message length
     * @return uint8_t  SUCCES or FAILURE
     */
    uint8_t forwarduTESLA(uint8_t *buffer, uint8_t size);

    /**
     * @brief Start neighbor handshake in neighbor discovery process. Sends protected nonce and expects it back incremented.
     * 
     * @param node_id   ID of neighbor node
     * @return uint8_t  SUCCESS or FAIL
     */
    uint8_t neighborHandshake(uint8_t node_id);

    /**
     * @brief Respond to neighbor handshake started by other node. Increments the nonce and sends it back.
     * 
     * @return uint8_t SUCCESS or FAIL
     */
    uint8_t neighborHandshakeResponse();
#ifdef ENABLE_UTESLA
    uTeslaClient    m_utesla;       // uTESLA class for ordinary node (not a BS)
#endif
#endif

public:
#ifdef __linux__
    /**
     * @brief BS constructor
     * 
     * @param slave_path    Path to slave JeeLink device
     * @param key_file      Path to file with keys generated by configurator
     */
    ProtectLayer(std::string &slave_path, std::string &key_file);    // throws runtime_error if there is a problem with key file

    /**
     * @brief Destructor
     * 
     */
    virtual ~ProtectLayer();

    /**
     * @brief Broadcast authenticated message to whole network using uTESLA protocol
     * 
     * @param buffer    Data to be broadcasted
     * @param size      Size of the data
     * @return uint8_t  SUCCESS or FAIL
     */
    uint8_t broadcastMessage(uint8_t *buffer, uint8_t size);

    /**
     * @brief Broadcast uTESLA key for previous round and start a new one
     * 
     * @return uint8_t  SUCCESS or FAIL
     */
    uint8_t broadcastKey();

    /**
     * @brief Send a message to a single node within one hop protected by encryption and MAC
     * 
     * @param msg_type  Type of the message (MSG_APP or MSG_OTHER)
     * @param receiver  Recipient
     * @param buffer    Data to be sent
     * @param size      Size of the data
     * @return uint8_t  SUCCESS or FAIL
     */
    uint8_t sendTo(msg_type_t msg_type, uint8_t receiver, uint8_t *buffer, uint8_t size);
#else

    /**
     * @brief Node onstructor
     * 
     */
    ProtectLayer();

    /**
     * @brief Send message to another node protected by encryption and MAC with pairwise key shared with node
     * 
     * @param msg_type  Type of the message
     * @param receiver  Receiver
     * @param buffer    Data to be sent
     * @param size      Size of the data
     * @return uint8_t  SUCCESS on success, FAIL on failure
     */
    uint8_t sendTo(msg_type_t msg_type, uint8_t receiver, uint8_t *buffer, uint8_t size);

    /**
     * @brief Send message to a CTP parent
     * 
     * @param msg_type  Type of the message
     * @param buffer    Data to be sent
     * @param size      Size of the data
     * @return uint8_t  SUCCESS on success, FAIL on failure
     */
    uint8_t sendCTP(msg_type_t msg_type, uint8_t *buffer, uint8_t size);

    /**
     * @brief Send data to BS. Method depends on msg_type - MSG_APP sends it directly in one hop, MSG_FORWARD uses CTP routing to forward a message.
     * 
     * @param msg_type  Type of the message. MSG_APP - send directly, MSG_FORWARD - send to CTP parent
     * @param buffer    Data to be sent
     * @param size      Size of the data
     * @return uint8_t  SUCCESS on success, FAIL on failure
     */
    uint8_t sendToBS(msg_type_t msg_type, uint8_t *buffer, uint8_t size);

#ifdef ENABLE_CTP
    /**
     * @brief Forward message to BS through CTP parent without any modification
     * 
     * @param buffer   
     * @param size      Size of the data
     * @return uint8_t  SUCCESS on success, FAIL on failure
     */
    uint8_t forwardToBS(uint8_t *buffer, uint8_t size);
#endif //ENABLE_CTP

    /**
     * @brief Receive message intended for this node.
     * 
     * @param buffer        Received already decrypted message.
     * @param buff_size     Size of the buffer for received message
     * @param received_size Size of the received message
     * @param timeout       Time in milliseconds to wait for a message
     * @return uint8_t      SUCCESS on success, FAIL on failure or FORWARD if the message was intended for BS
     */
    uint8_t receive(uint8_t *buffer, uint8_t buff_size, uint8_t *received_size, uint16_t timeout);

#ifdef ENABLE_UTESLA
    /**
     * @brief Verify uTESLA message
     * 
     * @param data      Message to be verified (including header)
     * @param data_size Size of the message
     * @return uint8_t  SUCCESS or FAIL
     */
    uint8_t verifyMessage(uint8_t *data, uint8_t data_size);
#endif // ENABLE_UTESLA

    /**
     * @brief Discover neighbors that the node shares a key with. If DELETE_KEYS is defined, other keys will be removed from EEPROM.
     * 
     * @return uint8_t SUCCESS or FAIL
     */
    uint8_t discoverNeighbors();

    /**
     * @brief Get the list of available neighbors. Returns 32-bit value, each bit indicating single node's presence.
     * 
     * @return uint32_t List of one-hop neighbors
     */
    uint32_t getNeighbors();

#endif  // __linux__

    /**
     * @brief Perform CTP establishment
     * 
     * @return uint8_t SUCCESS or FAIL
     */
    uint8_t startCTP();

    /**
     * @brief Receive message from other node or BS. The message is decrypted if needed. uTESLA messages are not modified. Blocking function.
     * 
     * @param buffer        Received data in plaintext.
     * @param buff_size     Bufer size for the received data
     * @param received_size Actual size of the received data
     * @return uint8_t      SUCCESS on success, FAIL on failure or FORWARD if the message was intended for BS
     */
    uint8_t receive(uint8_t *buffer, uint8_t buff_size, uint8_t *received_size);
    
    /**
     * @brief Get the node ID
     * 
     * @return uint8_t Node ID
     */
    uint8_t getNodeID();
};

#endif //  PROTECTLAYER_H