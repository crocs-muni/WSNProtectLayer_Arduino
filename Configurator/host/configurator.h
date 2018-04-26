#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <vector>
#include <string>
#include <fstream>

#include <stdint.h>

struct Key {
    std::string device;
    std::vector<uint8_t> buffer;
};

class Configurator {
private:
    // int m_nodes_num;    // not including BS
    int m_key_size;
    int m_keys_num;
    std::vector<Key> m_keys;

    bool generateKey(Key &key, const std::string &device, std::ifstream &random_file);
    bool readHeader(std::ifstream &input_file);
    bool readKey(std::ifstream &input_file, Key &key);
    bool writeHeader(std::ofstream &output_file);
    bool writeKey(std::ofstream &output_file, const Key &key);
    bool loadFromFile(const std::string filename);
public:
    // generate
    Configurator(std::string &in_filename, const int key_size);
    // load from file
    Configurator(std::string &in_filename);

    bool saveToFile(const std::string filename);
    bool upload();
};

#endif //  CONFIGURATOR_H