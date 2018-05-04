/**
 * @brief Class handling CTP establishment
 * 
 * @file    CTP.h
 * @author  Martin Sarkany
 * @date    05/2018
 */

#ifndef CPT_H
#define CTP_H

#include "common.h"
#include "ProtectLayerGlobals.h"

// #undef __linux__ // TODO!!! REMOVE - just for syntax highlighting in VS Code

#ifdef __linux__

/**
 * @brief CTP class
 * 
 */
class CTP {
private:
    int m_slave_fd; // file descriptor of slave serial device
public:
    /**
     * @brief Set the slave file descriptor
     * 
     * @param slave_fd  File descriptor
     */
    void setSlaveFD(int slave_fd);

    /**
     * @brief Perform CTP establishment
     * 
     * @param duration  Duration
     * @return uint8_t  SUCCESS or FAIL
     */
    uint8_t startCTP(uint32_t duration);
};

#else

/**
 * @brief CTP class
 * 
 */
class CTP {
private:
    uint8_t m_node_id;      // own ID
    uint8_t m_parent_id;    // parent's ID
    uint8_t m_distance;     // shortest distance
    uint8_t m_req_ack;      // true if communication requires acknowledgements, false otherwise

    /**
     * @brief Update distance and parent
     * 
     * @param message   Message content
     */
    void update(uint8_t *message);

    /**
     * @brief Wait for distance messages and handle them
     * 
     * @param end   Termination time
     */
    void handleDistanceMessages(uint32_t end);

    /**
     * @brief Broadcast own distance
     * 
     */
    void broadcastDistance();
public:
    /**
     * @brief Constructor
     * 
     */
    CTP();

    /**
     * @brief Set own ID
     * 
     * @param node_id   ID
     */
    void setNodeID(uint8_t node_id);

    /**
     * @brief Start CTP establishment
     * 
     * @param duration  Duration
     * @return uint8_t  SUCCESS or FAIL
     */
    uint8_t startCTP(uint32_t duration);
    
    // void send(uint8_t *buffer, uint8_t length);

    /**
     * @brief Get parent's ID
     * 
     * @return uint8_t  Parent's ID
     */
    uint8_t getParentID();
};

#endif

#endif //  CPT_H