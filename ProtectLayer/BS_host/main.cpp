#include "ProtectLayer.h"

#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    if(argc < 3){
        cerr << "Please specify path to keyfile" << endl;
        return 1;
    }

    string dev_path = argv[1];
    string key_path = argv[2];
    // ProtectLayer protect_layer(std::string(argv[1]), std::string(argv[2]));
    ProtectLayer protect_layer(dev_path, key_path);
    
    return 0;
}
