#include "configurator.h"

#include <iostream>

#include <cstdlib>
#include <getopt.h>

using namespace std;

void printHelp(const char *appname)
{
    cout << "Usage:" << endl
        << appname << " -c config_file -k key_size" << endl
        << "Note: config file contains device paths" << endl;
}


// TODO usage: either generate or load from file
int main(int argc, char **argv)
{
    char    c           = 0;
    string  in_filename;
    string  out_filename;
    int     key_size    = 0;
    bool    generate    = false;
    bool    save        = false;    // TODO
    bool    load        = false;    // TODO
    bool    upload      = false;    // TODO

    while ((c = getopt (argc, argv, "g:k:l:s:uh")) != -1){
        switch (c){
        case 'g':
            generate = true;
            in_filename = optarg;
            break;
        case 'k':
            key_size = atoi(optarg);
            break;
        case 'l':
            load = true;
            in_filename = optarg;
            break;
        case 's':
            save = true;
            out_filename = optarg;
            break;
        case 'u':
            upload = true;
            break;
        case 'h':
        case '?':
            printHelp(argv[0]);
            exit(0);
        default:
            abort ();
        }
    }

    if(in_filename.empty()){
        cerr << "Input file must be specified (-g or -l)" << endl;
        printHelp(argv[0]);
        exit(1);
    }

    if(load){
        if(generate){
            cerr << "Both -g and -l cannot be entered at the same time" << endl;
            exit(3);
        }
        if(key_size){
            cerr << "Key size cannot be specified when loading keys from file" << endl;
            exit(4);
        }
    }

    if(generate && !key_size){
        cerr << "Key size must be specified" << endl;
        exit(5);
    }

    try{
        Configurator configurator(in_filename, key_size);

        if(save){
            if(!configurator.saveToFile(out_filename)){
                cerr << "Failed to save keys to file" << endl;
                exit(7);
            }
        }

        if(upload){
            if(!configurator.upload()){
                cerr << "Failed to upload configuration" << endl;
                exit(10);
            }
        }
    } catch(exception &ex){
        cerr << ex.what() << endl;
        exit(20);
    }

    return 0;
}