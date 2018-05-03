#include "ProtectLayer.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "cstring"

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

        uint8_t snd_buff[1024] = "test message";
        uint8_t snd_len = strlen((const char*)snd_buff) + 1;

        while(true){
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if(protect_layer.broadcastMessage(snd_buff, snd_len) != SUCCESS){
                cerr << "Broadcast failed" << endl;
                return 17;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            if(protect_layer.broadcastKey() != SUCCESS){
                cerr << "Key announcement failed" << endl;
                return 27;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
    } catch(runtime_error &ex){
        cerr << ex.what();
        cerr.flush();
        return 15;
    }
    
    return 0;
}
