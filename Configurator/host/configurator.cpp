#include "configurator.h"
#include "conf_common.h"

#include "AES_crypto.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <chrono>

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_MESSAGE_LENGTH  256

bool Configurator::generateBSKey(Node &node, const uint8_t ID, const std::string &device, std::ifstream &random_file)
{
    uint8_t key_value[MAX_KEY_SIZE];
    node.ID = ID;
    node.device = device;
    node.BS_key.resize(m_key_size);
    random_file.read(reinterpret_cast<char*>(key_value), m_key_size);
    node.BS_key.assign(key_value, key_value + m_key_size);

    return true;
}

bool Configurator::generatePairwiseKeys(std::ifstream &random_file)
{
    uint8_t key_value[MAX_KEY_SIZE];

    m_pairwise_keys.resize(m_nodes_num);

    for(int i=0;i<m_nodes_num;i++){
        for(int j=0;j<i;j++){
            random_file.read(reinterpret_cast<char*>(key_value), m_key_size);
            if(random_file.fail()){
                return false;
            }
            m_pairwise_keys[i].push_back(std::vector<uint8_t>(key_value, key_value + m_key_size));
        }
    }

    return true;
}


bool Configurator::generateuTESLAKeys(std::ifstream &random_file)
{
    AES aes;
    AEShash hash(&aes);

    uint8_t tmp_hash[MAX_KEY_SIZE];
    random_file.read(reinterpret_cast<char*>(m_uTESLA_key), m_key_size);
    memcpy(tmp_hash, m_uTESLA_key, m_key_size);

    for(int i=1;i<m_uTESLA_rounds + 1;i++){
        if(!hash.hash(tmp_hash, m_key_size, m_uTESLA_last_element, MAX_KEY_SIZE)){
            std::cerr << "Failed to compute hash" << std::endl;
            return false;
        }
        memcpy(tmp_hash, m_uTESLA_last_element, m_key_size);
    }

    return true;
}

// also cuts the device name
bool Configurator::readID(std::string &line, int *id)
{
    std::string delimiter = " \t";

    size_t pos = 0;
    if((pos = line.find_last_of(delimiter)) != std::string::npos) {
        std::string str_id = line.substr(pos + 1);
        try{
            *id = std::stoi(str_id);
        } catch(std::invalid_argument &ex){
            return false;
        }
        line = line.substr(0, pos);

        return true;
    }

    return false;
}

Configurator::Configurator(std::string &in_filename, const int uTESLA_rounds, const int key_size):
m_key_size(key_size), m_uTESLA_rounds(uTESLA_rounds)
{
    if(key_size){
        m_nodes_num = 0;

        std::ifstream paths_file(in_filename.c_str(), std::ifstream::in);

        if(!paths_file.is_open()){
            std::stringstream err;
            err << "file " << in_filename << " does not exist" << std::endl;
            throw std::runtime_error(err.str());
        }

        std::ifstream random_file("/dev/urandom", std::ifstream::in);
        if(!paths_file.is_open()){
            throw std::runtime_error("Failed to open /dev/urandom");
        } 

        Node node;
        std::string device_name;
        while(getline(paths_file, device_name)){
            if(device_name.empty() || !(device_name.find_first_not_of(" \t") != std::string::npos)){
                continue;
            }

            int node_id = 0;
            if(!readID(device_name, &node_id)){
                throw std::runtime_error("Device ID not supplied for " + device_name);
            }
            
            if(!generateBSKey(node, node_id, device_name, random_file)){
                throw std::runtime_error("Failed to generate BS key");
            }
            
            m_nodes_num++;
            m_nodes.push_back(node);
        }

        if(!generatePairwiseKeys(random_file)){
            throw std::runtime_error("Failed to generate random keys");
        }

        if(!generateuTESLAKeys(random_file)){
            throw std::runtime_error("Failed to generate random keys");
        }

        paths_file.close();
        random_file.close();


        if((m_nodes_num = m_nodes.size()) < 1){
            throw std::runtime_error("No devices in config file");
        }
    } else {
        if(!loadFromFile(in_filename)){
            throw std::runtime_error("Failed to load keys from file");
        }
    }
}

bool Configurator::writeHeader(std::ofstream &output_file)
{
    output_file.write(reinterpret_cast<char*>(&m_nodes_num), sizeof(m_nodes_num));
    if(output_file.fail()){
        std::cerr << "Failed to write number of keys" << std::endl;
        return false;
    }

    output_file.write(reinterpret_cast<char*>(&m_key_size), sizeof(m_key_size));
    if(output_file.fail()){
        std::cerr << "Failed to write node size" << std::endl;
        return false;
    }

    output_file.flush();
    return true;
}

bool Configurator::writeNode(std::ofstream &output_file, const Node &node)
{
    int dev_name_size = node.device.length() + 1;
    output_file.write(reinterpret_cast<char*>(&dev_name_size), sizeof(int));
    if(output_file.fail()){
        std::cerr << "Failed to write device name length" << std::endl;
        return false;
    }

    output_file.write(node.device.c_str(), dev_name_size);    
    if(output_file.fail()){
        std::cerr << "Failed to write device name" << std::endl;
        return false;
    }

    output_file.write(reinterpret_cast<const char*>(&node.ID), sizeof(node.ID));
    if(output_file.fail()){
        std::cerr << "Failed to write node ID" << std::endl;
        return false;
    }

    output_file.write(reinterpret_cast<const char*>(node.BS_key.data()), node.BS_key.size());
    if(output_file.fail()){
        std::cerr << "Failed to write node value" << std::endl;
        return false;
    }

    output_file.flush();

    return true;
}


bool Configurator::writeuTESLAKeys(std::ofstream &output_file)
{
    output_file.write(reinterpret_cast<const char*>(m_uTESLA_key), m_key_size);
    if(output_file.fail()){
        std::cerr << "Failed to write first uTESLA key" << std::endl;
        return false;
    }
    
    output_file.write(reinterpret_cast<const char*>(m_uTESLA_last_element), m_key_size);
    if(output_file.fail()){
        std::cerr << "Failed to write last uTESLA key" << std::endl;
        return false;
    }

    return true;
}

bool Configurator::saveToFile(const std::string filename)
{
    std::ofstream output_file(filename, std::ios::binary);
    if(!output_file.is_open()){
        std::cerr << "Failed to open output file" << std::endl;
        return false;
    }

    if(!writeHeader(output_file)){
        std::cerr << "Failed to write header" << std::endl;
        output_file.close();
        return false;
    }

    for(std::vector<Node>::iterator it = m_nodes.begin(); it != m_nodes.end(); it++){
        if(!writeNode(output_file, *it)){
            std::cerr << "Failed to write keys" << std::endl;
            output_file.close();
            return false;
        }
    }

    if(!writePairwiseKeys(output_file)){
        std::cerr << "Failed to write pairwise keys" << std::endl;
        output_file.close();
        return false;
    }

    output_file.close();

    return true;
}

bool Configurator::readHeader(std::ifstream &input_file)
{
    input_file.read(reinterpret_cast<char*>(&m_nodes_num), sizeof(m_nodes_num));
    if(input_file.fail()){
        std::cerr << "Failed to read number of keys" << std::endl;
        return false;
    }
    
    input_file.read(reinterpret_cast<char*>(&m_key_size), sizeof(m_key_size));
    if(input_file.fail()){
        std::cerr << "Failed to read node size" << std::endl;
        return false;
    }

    return true;
}

bool Configurator::readNode(std::ifstream &input_file, Node &node)
{
    int dev_name_size = node.device.length() + 1;
    char device_name[256];  // TODO define for max device name length

    input_file.read(reinterpret_cast<char*>(&dev_name_size), sizeof(int));
    if(input_file.fail()){
        std::cerr << "Failed to read device name length" << std::endl;
        return false;
    }

    memset(device_name, 0, 256);    // TODO use define for max device name length
    input_file.read(device_name, dev_name_size);    
    if(input_file.fail()){
        std::cerr << "Failed to read device name" << std::endl;
        return false;
    }
    node.device = device_name;

    input_file.read(reinterpret_cast<char*>(&node.ID), sizeof(node.ID));
    if(input_file.fail()){
        std::cerr << "Failed to read node ID" << std::endl;
        return false;
    }

    // reusing device name buffer for node value
    memset(device_name, 0, 256);    // TODO use define for max device name length
    input_file.read(device_name, m_key_size);
    if(input_file.fail()){
        std::cerr << "Failed to read node value" << std::endl;
        return false;
    }

    node.BS_key.assign(device_name, device_name + m_key_size);

    return true;
}

bool Configurator::readuTESLAKeys(std::ifstream &input_file)
{
    input_file.read(reinterpret_cast<char*>(m_uTESLA_key), m_key_size);
    if(input_file.fail()){
        std::cerr << "Failed to read first uTESLA key" << std::endl;
        return false;
    }

    input_file.read(reinterpret_cast<char*>(m_uTESLA_key), m_key_size);
    if(input_file.fail()){
        std::cerr << "Failed to read last uTESLA key" << std::endl;
        return false;
    }

    return true;
}

bool Configurator::loadFromFile(const std::string filename)
{
    std::ifstream input_file(filename, std::ios::binary);
    if(!input_file.is_open()){
        std::cerr << "Failed to open input file" << std::endl;
        return false;
    }

    if(!readHeader(input_file)){
        std::cerr << "Failed to read header" << std::endl;
        input_file.close();
        return false;
    }

    for(int i=0;i<m_nodes_num;i++){
        Node node;

        if(!readNode(input_file, node)){
            std::cerr << "Failed to read node " << i << std::endl;
            input_file.close();
            return false;
        }

        m_nodes.push_back(node);
    }

    if(static_cast<size_t>(m_nodes_num) != m_nodes.size()){
        std::cerr << "File corrupted - not enough keys found" << std::endl;
        input_file.close();
        return false;
    }

    if(!readPairwiseKeys(input_file)){
        std::cerr << "Failed to read pairwise keys" << std::endl;
        input_file.close();
        return false;
    }

    input_file.close();

    return true;
}

bool Configurator::readPairwiseKeys(std::ifstream &input_file)
{
    char key[MAX_KEY_SIZE];

    m_pairwise_keys.resize(m_nodes_num);

    for(int i=0;i<m_nodes_num;i++){
        for(int j=0;j<i;j++){
            input_file.read(key, m_key_size);
            if(input_file.fail()){
                std::cerr << "Failed to read device name length" << std::endl;
                return false;
            }
            m_pairwise_keys[i].push_back(std::vector<uint8_t>(key, key + m_key_size));
        }  
    }

    return true;
}

bool Configurator::writePairwiseKeys(std::ofstream &output_file)
{
    for(int i=0;i<m_nodes_num;i++){
        for(int j=0;j<i;j++){
            output_file.write(reinterpret_cast<char*>(m_pairwise_keys[i][j].data()), m_key_size);
            if(output_file.fail()){
                std::cerr << "Failed to write pairwise key" << std::endl;
                return false;
            }
        }
    }
    
    output_file.flush();
    
    return true;
}


// stolen from stackoverflow
int set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
            std::cerr << "error " << errno << " from tcgetattr" << std::endl;
            return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 10;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0){
            std::cerr << "error " << errno << " from tcsetattr: " << std::endl;
            return -1;
    }
    return 0;
}

void set_blocking (int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0){
            std::cerr << "error " << errno << " from tggetattr" << std::endl;
            return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 30;

    if (tcsetattr (fd, TCSANOW, &tty) != 0){
         std::cerr << "error " << errno << " setting term attributes" << std::endl;
    }
}

int openSerialPort(std::string path)
{
    int serial_fd = open(path.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd < 0){
        std::cerr << "Failed to open serial port " << path << ", errno: " << errno << std::endl;
        return serial_fd;
    }
    set_interface_attribs(serial_fd, B115200, 0);
    set_blocking(serial_fd, 0); // set not blocking

    return serial_fd;
}

bool checkResponse(int fd)
{
    uint8_t buffer[MAX_MESSAGE_LENGTH];
    int rval;

    if((rval = read(fd, buffer, MAX_MESSAGE_LENGTH)) < 1){
        // wait and try again
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        if((rval = read(fd, buffer, MAX_MESSAGE_LENGTH)) < 1){
            std::cerr << "Failed to receive response from the node" << std::endl;
            return false;
        }
    }

    if(rval != 1){
        std::cout << "Received more data than expected (" << rval << "):" << std::endl;
        for(int i=0;i<rval;i++){
            printf("%02X ", buffer[i]);
        }
        std::cout << std::endl;
        std::cout.flush();
    }

    if(buffer[0] == REPLY_OK || buffer[0] == REPLY_DONE){
        return true;
    } else {
        switch(buffer[0]){
            case REPLY_ERR_LEN:
                std::cerr << "Wrong message length" << std::endl;
                return false;
            case REPLY_ERR_DONE:
                std::cerr << "Configuration already done, node does not accept further data" << std::endl;
                return false;
            case REPLY_ERR_MSG_SIZE:
                std::cerr << "Did not receive the whole message" << std::endl;
                return false;
            case REPLY_ERR_EEPROM:
                std::cerr << "EEPROM failure" << std::endl;
                return false;
            case REPLY_ERR_MSG_TYPE:
                std::cerr << "Wrong message type" << std::endl;
                return false;
            default:
                std::cerr << "Unknown error" << std::endl;
                return false;
        }
    }        
}

// bool Configurator::requestKey(std::string device)
// {
//     // TODO open port
//     // requestKey(fd);
//     // TODO close port
//     return true;
// }

bool Configurator::requestKey(int fd, uint8_t node_id)
{
    uint8_t buffer[32];

    memset(buffer, 0, 32);
    buffer[0] = 2;
    buffer[1] = 2;
    buffer[2] = MSG_REQ_KEY;
    buffer[3] = node_id;

    write(fd, buffer, 4);
    tcdrain(fd);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int rsplen = read(fd, buffer, 32);
    if(rsplen < 0){
        std::cerr << "Failed to read key" << std::endl;
        return false;
    }
    if(rsplen < 16){
        std::cerr << "Response too short (" << rsplen <<")" << std::endl;
        return false;
    }

    for(int i=0;i<rsplen;i++){
        printf("%02X ", buffer[i]);
    }
    std::cout << std::endl;
    std::cout.flush();

    return true;
}

bool Configurator::uploadSingle(const Node &node, int node_index)   // TODO remove node, keep index
{
#ifdef DEBUG
    std::cout << std::endl << "Configuring " << node.device << std::endl;
#endif

    int fd = openSerialPort(node.device);
    if(fd < 0){
        std::cerr << "Failed to open serial port " << node.device << std::endl;
        return false;
    }

    uint8_t message_buffer[MAX_MESSAGE_LENGTH];
    int rval;


    read(fd, message_buffer, MAX_MESSAGE_LENGTH);
    // upload node ID
    memset(message_buffer, 0, MAX_MESSAGE_LENGTH);
    message_buffer[0] = MSG_TYPE_SIZE + sizeof(node.ID);
    message_buffer[1] = message_buffer[0];
    message_buffer[2] = MSG_ID;
    memcpy(message_buffer + 3, &node.ID, sizeof(node.ID));
    if((rval = write(fd, message_buffer, message_buffer[0] + 2)) != message_buffer[0] + 2){
        std::cerr << "Failed to write ID to node " << node.device << std::endl;
        if(rval > -1){
            std::cerr << rval << " bytes were written" << std::endl;
        }
        close(fd);
        return false;
    }
    tcdrain(fd);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if(!checkResponse(fd)){
        std::cerr << "Failed to configure ID for " << node.device << std::endl;
        close(fd);
        return false;
    }


    read(fd, message_buffer, MAX_MESSAGE_LENGTH);
    // upload BS key
    memset(message_buffer, 0, MAX_MESSAGE_LENGTH);
    message_buffer[0] = MSG_TYPE_SIZE + m_key_size;
    message_buffer[1] = message_buffer[0];
    message_buffer[2] = MSG_BS_KEY;
    memcpy(message_buffer + 3, node.BS_key.data(), m_key_size);
    if((rval = write(fd, message_buffer, message_buffer[0] + 2)) != message_buffer[0] + 2){
        std::cerr << "Failed to write BS key to node " << node.device << std::endl;
        if(rval > -1){
            std::cerr << rval << " bytes were written" << std::endl;
        }
        close(fd);
        return false;
    }
    tcdrain(fd);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if(!checkResponse(fd)){
        std::cerr << "Failed to configure BS key for " << node.device << std::endl;
        close(fd);
        return false;
    }

    // upload pairwise keys
    memset(message_buffer, 0, MAX_MESSAGE_LENGTH);
    message_buffer[0] = MSG_TYPE_SIZE + 1 /*node ID*/ + m_key_size;
    message_buffer[1] = message_buffer[0];
    message_buffer[2] = MSG_NODE_KEY;

    for(int i=0;i<m_nodes_num;i++){
        // skip itself
        if(i != node_index){
            message_buffer[3] = m_nodes[i].ID;
            if(node_index < i){
    #ifdef DEBUG
                std::cout << "Uploading key [" << i << "][" << node_index << "]" << std::endl;
    #endif
                memcpy(message_buffer + 4, m_pairwise_keys[i][node_index].data(), m_key_size);
            } else {
    #ifdef DEBUG
                std::cout << "Uploading key [" << node_index << "][" << i << "]" << std::endl;
    #endif
                memcpy(message_buffer + 4, m_pairwise_keys[node_index][i].data(), m_key_size);
            }

            if((rval = write(fd, message_buffer, message_buffer[0] + 2)) != message_buffer[0] + 2){
                std::cerr << "Failed to write pairwise key to node " << node.device << std::endl;
                if(rval > -1){
                    std::cerr << rval << " bytes were written" << std::endl;
                }
                close(fd);
                return false;
            }
            tcdrain(fd);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if(!checkResponse(fd)){
                std::cerr << "Failed to configure pairwise key for " << node.device << std::endl;
                close(fd);
                return false;
            }
        }
    }

    memset(message_buffer, 0, MAX_MESSAGE_LENGTH);
    message_buffer[0] = MSG_TYPE_SIZE + m_key_size;
    message_buffer[1] = message_buffer[0];
    message_buffer[2] = MSG_UTESLA_KEY;
    memcpy(message_buffer + 3, m_uTESLA_last_element, m_key_size);
    if((rval = write(fd, message_buffer, message_buffer[0] + 2)) != message_buffer[0] + 2){
        std::cerr << "Failed to write uTESLA key to node " << node.device << std::endl;
        if(rval > -1){
            std::cerr << rval << " bytes were written" << std::endl;
        }
        close(fd);
        return false;
    }
    tcdrain(fd);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if(!checkResponse(fd)){
        std::cerr << "Failed to configure uTESLA key for " << node.device << std::endl;
        close(fd);
        return false;
    }


#ifdef DEBUG
    if(m_nodes_num > 1){
        uint8_t req_key_node_index = m_nodes_num - 1;
        if(node_index == req_key_node_index){
            req_key_node_index--;
        }
        std::cout << "Requesting key for node " << (int) req_key_node_index << " from node "<< node_index << std::endl;
        if(!requestKey(fd, m_nodes[req_key_node_index].ID)){
            std::cerr << "Failed to read key that was set" << std::endl;
            return false;
        }
    } else {
        std::cout << "Not requesting key to verify because there is only 1 node" << std::endl;// TODO req BS key
    }
#endif

    close(fd);

    return true;
}

bool Configurator::upload()
{
    for(int i=0;i<m_nodes_num;i++){
        if(!uploadSingle(m_nodes[i], i)){
            std::cerr << "Failed to upload config for node " << m_nodes[i].device << std::endl;
            return false;
        }
    }

    return true;
}

std::vector<Node> Configurator::getNodes()
{
    return m_nodes;
}

int Configurator::getKeySize()
{
    return m_key_size;
}


const uint8_t* Configurator::getuTESLAKey()
{
    return m_uTESLA_key;
}

const uint8_t* Configurator::getuTESLALastElement()
{
    return m_uTESLA_last_element;
}