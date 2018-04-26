#include "configurator.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <cstring>

#define MAX_KEY_SIZE    128

bool Configurator::generateKey(Key &key, const std::string &device, std::ifstream &random_file)
{
    uint8_t key_value[MAX_KEY_SIZE];
    key.device = device;
    key.buffer.resize(m_key_size);
    random_file.read(reinterpret_cast<char*>(key_value), m_key_size);
    key.buffer.assign(key_value, key_value + m_key_size);

    return true;
}

Configurator::Configurator(std::string &in_filename, const int key_size):
m_key_size(key_size)
{
    if(key_size){
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

        // BS
        Key key;
        const std::string BS_name = "BS";
        generateKey(key, BS_name, random_file);

        m_keys_num++;
        m_keys.push_back(key);

        std::string device_name;
        while(getline(paths_file, device_name)){
            if(device_name.empty() || !(device_name.find_first_not_of(" \t") != std::string::npos)){
                continue;
            }
            generateKey(key, device_name, random_file);
            
            m_keys_num++;
            m_keys.push_back(key);
        }

        paths_file.close();
        random_file.close();


        if((m_keys_num = m_keys.size()) < 2){
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
        std::cerr << "Failed to write key size" << std::endl;
        return false;
    }

    output_file.flush();
    return true;
}

bool Configurator::writeKey(std::ofstream &output_file, const Key &key)
{
    int dev_name_size = key.device.length() + 1;
    output_file.write(reinterpret_cast<char*>(&dev_name_size), sizeof(int));
    if(output_file.fail()){
        std::cerr << "Failed to write device name length" << std::endl;
        return false;
    }

    output_file.write(key.device.c_str(), dev_name_size);    
    if(output_file.fail()){
        std::cerr << "Failed to write device name" << std::endl;
        return false;
    }

    output_file.write(reinterpret_cast<const char*>(key.buffer.data()), key.buffer.size());
    if(output_file.fail()){
        std::cerr << "Failed to write key value" << std::endl;
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
        return false;
    }

    for(std::vector<Key>::iterator it = m_keys.begin(); it != m_keys.end(); it++){
        if(!writeKey(output_file, *it)){
            std::cerr << "Failed to write keys" << std::endl;
            return false;
        }
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
        std::cerr << "Failed to read key size" << std::endl;
        return false;
    }

    return true;
}

bool Configurator::readKey(std::ifstream &input_file, Key &key)
{
    int dev_name_size = key.device.length() + 1;
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
    key.device = device_name;

    // reusing device name buffer for key value
    memset(device_name, 0, 256);    // TODO use define for max device name length
    input_file.read(device_name, m_key_size);
    if(input_file.fail()){
        std::cerr << "Failed to read key value" << std::endl;
        return false;
    }

    key.buffer.assign(device_name, device_name + m_key_size);

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
        return false;
    }

    for(int i=0;i<m_keys_num;i++){
        Key key;

        if(!readKey(input_file, key)){
            std::cerr << "Failed to read key " << i << std::endl;
        }

        m_keys.push_back(key);
    }

    if(static_cast<size_t>(m_keys_num) != m_keys.size()){
        std::cerr << "File corrupted - not enough keys found" << std::endl;
        return false;
    }

    return true;
}

bool Configurator::upload()
{
    return false;
}