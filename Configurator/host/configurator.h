/**
 * @brief Configurator for JeeLink devices to set nodes' IDs, crypto keys, etc.
 * 
 * @file    configurator.h
 * @author  Martin Sarkany
 * @date    05/2018
 */

#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include "ProtectLayerGlobals.h"

#include <vector>
#include <string>
#include <fstream>

#include <stdint.h>

#ifndef MAX_KEY_SIZE
#define MAX_KEY_SIZE        64
#endif


struct Node {
    uint8_t              ID;        // node's ID
    std::string          device;    // device path
    std::vector<uint8_t> BS_key;    // pairwise key with BS
};

/**
 * @brief Configurator class
 * 
 */
class Configurator {
private:
    int                 m_key_size;                             // key size
    int                 m_nodes_num;                            // number of nodes (excluding BS)
    std::vector<Node>   m_nodes;                                // nodes
    std::vector< std::vector< std::vector <uint8_t> > > m_pairwise_keys;    // pairwise keys (excluding BS)
    int                 m_uTESLA_rounds;                        // number of uTESLA rounds
    uint8_t             m_uTESLA_key[MAX_KEY_SIZE];             // first uTESLA hash chain element
    uint8_t             m_uTESLA_last_element[MAX_KEY_SIZE];    // last uTESLA hash chain element

    /**
     * @brief Parse node's ID from configuration line and cut it out
     * 
     * @param line      Single line from configuration file
     * @param id        ID parsed from a line
     * @return true     Success
     * @return false    Failure
     */
    bool readID(std::string &line, int *id);

    /**
     * @brief Generate a pairwise key with a BS for a single node, set ID and path
     * 
     * @param node          Node structure
     * @param ID            Node's ID
     * @param device        Path to device
     * @param random_file   /dev/urandom file descriptor
     * @return true         Success
     * @return false        Failure
     */
    bool generateBSKey(Node &node, const uint8_t ID, const std::string &device, std::ifstream &random_file);

    /**
     * @brief Generate pairwise keys for nodes (excluding BS)
     * 
     * @param random_file   /dev/urandom file descriptor
     * @return true         Success
     * @return false        Failure
     */
    bool generatePairwiseKeys(std::ifstream &random_file);

    /**
     * @brief Generate first and last uTESLA hash chain elements
     * 
     * @param random_file   /dev/urandom file descriptor
     * @return true         Success
     * @return false        Failure
     */
    bool generateuTESLAKeys(std::ifstream &random_file);

    /**
     * @brief Read file header of stored keys
     * 
     * @param input_file    Input file descriptor
     * @return true         Success
     * @return false        Failure
     */
    bool readHeader(std::ifstream &input_file);

    /**
     * @brief Read BS key and ID for a single node
     * 
     * @param input_file    Input file descriptor
     * @param node          Node structure
     * @return true         Success
     * @return false        Failure
     */
    bool readNode(std::ifstream &input_file, Node &node);

    /**
     * @brief Read pairwise keys from an input file
     * 
     * @param input_file    Input file descriptor
     * @return true         Success
     * @return false        Failure
     */
    bool readPairwiseKeys(std::ifstream &input_file);

    /**
     * @brief Read uTESLA keys from an input file
     * 
     * @param input_file    Input file descriptor
     * @return true         Success
     * @return false        Failure
     */
    bool readuTESLAKeys(std::ifstream &input_file);

    /**
     * @brief Write file header
     * 
     * @param output_file   Output file
     * @return true         Success
     * @return false        Failure
     */
    bool writeHeader(std::ofstream &output_file);

    /**
     * @brief Write node's path, ID and BS key
     * 
     * @param output_file   Output file
     * @param node          Node structure
     * @return true         Success
     * @return false        Failure
     */
    bool writeNode(std::ofstream &output_file, const Node &node);

    /**
     * @brief Write pairwise keys to file
     * 
     * @param output_file   Output file
     * @return true         Success
     * @return false        Failure
     */
    bool writePairwiseKeys(std::ofstream &output_file);

    /**
     * @brief Write uTESLA keys to file
     * 
     * @param output_file   Output file
     * @return true         Success
     * @return false        Failure
     */
    bool writeuTESLAKeys(std::ofstream &output_file);

    /**
     * @brief Read everything from a file
     * 
     * @param filename      Input file
     * @return true         Success
     * @return false        Failure
     */
    bool loadFromFile(const std::string &filename);

    /**
     * @brief Upload configuration to a single node
     * 
     * @param node          Node structure
     * @param node_index    Node index
     * @return true         Success
     * @return false        Failure
     */
    bool uploadSingle(const Node &node, int node_index);

    /**
     * @brief Request a pairwise key from a node (useful to check correctness)
     * 
     * @param fd            File descriptor of a node
     * @param node_id       Node's ID
     * @return true         Success
     * @return false        Failure
     */
    bool requestKey(int fd, uint8_t node_id);
public:
    
    /**
     * @brief Constructor
     * 
     * @param in_filename   Configuration file in case if non-zero key size, key's file otherwise 
     * @param uTESLA_rounds Number of uTESLA rounds
     * @param key_size      Key size
     */
    Configurator(std::string &in_filename, const int uTESLA_rounds, const int key_size = 0);

    /**
     * @brief Save configuration to file
     * 
     * @param filename      Input file
     * @return true         Success
     * @return false        Failure
     */
    bool saveToFile(const std::string &filename);

    /**
     * @brief Upload configuration to nodes
     * 
     * @return true         Success
     * @return false        Failure
     */
    bool upload();

    /**
     * @brief Get all nodes structures
     * 
     * @return std::vector<Node> Nodes
     */
    std::vector<Node> getNodes();

    /**
     * @brief Get key size
     * 
     * @return int  Key size
     */
    int getKeySize();

    /**
     * @brief Get first hash chain element
     * 
     * @return const uint8_t* First hash chain element
     */
    const uint8_t *getuTESLAKey();

    /**
     * @brief Get last hash chain element
     * 
     * @return const uint8_t* Last hash chain element
     */
    const uint8_t *getuTESLALastElement();

    /**
     * @brief Get number of uTESLA rounds
     * 
     * @return int Number of uTESLA rounds
     */
    int getuTESLARounds();
};

#endif //  CONFIGURATOR_H