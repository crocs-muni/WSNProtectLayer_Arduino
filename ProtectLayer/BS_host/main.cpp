#include "ProtectLayer.h"

#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    if(argc < 2){
        cerr << "Please specify path to keyfile" << endl;
    }


    ProtectLayer protect_layer(string(argv[1]));
    return 0;
}
