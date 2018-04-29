#ifndef CPT_H
#define CTP_H

#include "common.h"
#include "ProtectLayerGlobals.h"

// #undef __linux__ // TODO!!! REMOVE - just for syntax highlighting in VS Code

#ifdef __linux__

class CTP {
private:
    int m_slave_fd;
public:
    // CTP();
    void setSlaveFD(int slave_fd);
    uint8_t startCTP(uint32_t duration);
};

#else

class CTP {
private:
    uint8_t m_node_id;
    uint8_t m_parent_id;
    uint8_t m_distance;
    uint8_t m_req_ack;

    void update(uint8_t *message);
    void handleDistanceMessages(uint32_t end);
    void broadcastDistance();
public:
    CTP();
    void setNodeID(uint8_t node_id);
    uint8_t startCTP(uint32_t duration);
    // void forward(/*?*/);
    void send(uint8_t *buffer, uint8_t length);

    uint8_t getParent();
};

#endif

#endif //  CPT_H