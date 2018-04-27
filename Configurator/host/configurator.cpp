#include "configurator.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <cstring>

#define MAX_KEY_SIZE    128

bool Configurator::generateBSKey(Node &node, const uint16_t ID, const std::string &device, std::ifstream &random_file)
{
    uint8_t key_value[MAX_KEY_SIZE];
    node.ID = ID;
    node.device = device;
    node.buffer.resize(m_key_size);
    random_file.read(reinterpret_cast<char*>(key_value), m_key_size);
    node.buffer.assign(key_value, key_value + m_key_size);

    return true;
}

bool Configurator::generatePairwiseKeys(std::ifstream &random_file)
{
    uint8_t key_value[MAX_KEY_SIZE];

    m_pairwise_keys.resize(m_keys_num);

    for(int i=0;i<m_keys_num;i++){
        for(int j=0;j<i;j++){
            // std::cout << "Generating key for pair [" << i << "][" << j << "]" << std::endl;
            random_file.read(reinterpret_cast<char*>(key_value), m_key_size);
            if(random_file.fail()){
                return false;
            }
            m_pairwise_keys[i].push_back(std::vector<uint8_t>(key_value, key_value + m_key_size));
        }
    }

    return true;
}

Configurator::Configurator(std::string &in_filename, const int key_size):
m_key_size(key_size)
{
    if(key_size){
        m_keys_num = 0;

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
        
        // BS
        // const std::string BS_name = "BS";
        // generateKey(node, BS_name, random_file);

        // m_keys_num++;
        // m_keys.push_back(node);

        std::string device_name;
        while(getline(paths_file, device_name)){
            if(device_name.empty() || !(device_name.find_first_not_of(" \t") != std::string::npos)){
                continue;
            }
            
            if(!generateBSKey(node, m_keys_num + 1, device_name, random_file)){
                throw std::runtime_error("Failed to generate BS key");
            }
            
            m_keys_num++;
            m_keys.push_back(node);
        }

        if(!generatePairwiseKeys(random_file)){
            throw std::runtime_error("Failed to generate random keys");
        }

        paths_file.close();
        random_file.close();


        if((m_keys_num = m_keys.size()) < 1){
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
    output_file.write(reinterpret_cast<char*>(&m_keys_num), sizeof(m_keys_num));
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

bool Configurator::writeBSKey(std::ofstream &output_file, const Node &node)
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

    output_file.write(reinterpret_cast<const char*>(node.buffer.data()), node.buffer.size());
    if(output_file.fail()){
        std::cerr << "Failed to write node value" << std::endl;
        return false;
    }

    output_file.flush();

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

    for(std::vector<Node>::iterator it = m_keys.begin(); it != m_keys.end(); it++){
        if(!writeBSKey(output_file, *it)){
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
    input_file.read(reinterpret_cast<char*>(&m_keys_num), sizeof(m_keys_num));
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

bool Configurator::readBSKey(std::ifstream &input_file, Node &node)
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

    // reusing device name buffer for node value
    memset(device_name, 0, 256);    // TODO use define for max device name length
    input_file.read(device_name, m_key_size);
    if(input_file.fail()){
        std::cerr << "Failed to read node value" << std::endl;
        return false;
    }

    node.buffer.assign(device_name, device_name + m_key_size);

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

    for(int i=0;i<m_keys_num;i++){
        Node node;

        if(!readBSKey(input_file, node)){
            std::cerr << "Failed to read node " << i << std::endl;
            input_file.close();
            return false;
        }

        m_keys.push_back(node);
    }

    if(static_cast<size_t>(m_keys_num) != m_keys.size()){
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

bool Configurator::upload()
{
    return false;
}


bool Configurator::readPairwiseKeys(std::ifstream &input_file)
{
    char key[MAX_KEY_SIZE];

    m_pairwise_keys.resize(m_keys_num);

    for(int i=0;i<m_keys_num;i++){
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
    for(int i=0;i<m_keys_num;i++){
        for(int j=0;j<i;j++){
            std::cout << "Writing key [" << i << "][" << j << "]" << std::endl; // TODO remove 
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
