#include "ProtectLayer.h"

#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    if(argc < 3){
        system("pwd");
        cerr << "Usage:" << endl 
             << argv[0] << " device_path key_file_path"  << endl;
        return 1;
    }

    string dev_path = argv[1];
    string key_path = argv[2];
    // ProtectLayer protect_layer(std::string(argv[1]), std::string(argv[2]));
    try {
        ProtectLayer protect_layer(dev_path, key_path);

        if(protect_layer.startCTP() != SUCCESS){
            cerr << "Failed to establish CTP tree" << endl;
            return 7;
        }
    } catch(runtime_error &ex){
        cerr << ex.what();
        return 15;
    }
    
    return 0;
}
