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
    
    try {
        ProtectLayer protect_layer(dev_path, key_path);

        if(protect_layer.startCTP() != SUCCESS){
            cerr << "Failed to establish CTP tree" << endl;
            cerr.flush();
            return 7;
        }

        uint8_t rcvd_buff[1024];
        uint8_t rcvd_len;

        while(true){
            if(protect_layer.receive(rcvd_buff, MAX_MSG_SIZE, &rcvd_len) == SUCCESS){
                printBufferHex(rcvd_buff, rcvd_len);
                // cout << "Received ";
                // printBufferHex(rcvd_buff + 3, rcvd_len - 3);
                // cout << " from " << (int) rcvd_buff[1] << ", message type " << (int) rcvd_buff[1] << endl;
            } else {
                cerr << "Receive failed" << endl;
            }
        }
    } catch(runtime_error &ex){
        cerr << ex.what();
        cerr.flush();
        return 15;
    }
    
    return 0;
}
