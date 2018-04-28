#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <vector>
#include <string>
#include <fstream>

#include <stdint.h>

struct Node {
    uint8_t              ID;
    std::string          device;
    std::vector<uint8_t> BS_key;
};

class Configurator {
private:
    // int m_nodes_num;    // not including BS
    int m_key_size;
    int m_nodes_num; // TODO rename to m_nodes_num
    std::vector<Node> m_nodes;
    std::vector< std::vector< std::vector <uint8_t> > > m_pairwise_keys;

    bool readID(std::string line, int *id);
    bool generateBSKey(Node &node, const uint8_t ID, const std::string &device, std::ifstream &random_file);
    bool generatePairwiseKeys(std::ifstream &random_file);
    bool readHeader(std::ifstream &input_file);
    bool readNode(std::ifstream &input_file, Node &key);
    bool readPairwiseKeys(std::ifstream &input_file);
    bool writeHeader(std::ofstream &output_file);
    bool writeNode(std::ofstream &output_file, const Node &key);
    bool writePairwiseKeys(std::ofstream &output_file);
    bool loadFromFile(const std::string filename);
    bool uploadSingle(const Node &node, int node_index);
    bool requestKey(int fd, uint8_t node_id);
public:
    // generate
    Configurator(std::string &in_filename, const int key_size);
    // load from file
    Configurator(std::string &in_filename);

    bool saveToFile(const std::string filename);
    bool upload();
};

#endif //  CONFIGURATOR_H