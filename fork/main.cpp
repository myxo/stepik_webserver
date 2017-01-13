#include "master_server.h"

#include <unistd.h>
#include <string>
#include <iostream>


void print_help_message(){
    std::cerr << "usage: ./final -h <ip> -p <port> -d <directory>\n";
}

int main(int argc, char *argv[]){
    int opt, flags = 0, port;
    std::string ip, directory;
    bool demonise_process = false;

    while((opt = getopt(argc, argv, "h:p:d:s")) != -1){
        switch(opt){
            case 'h': ip = optarg;          flags++; break;
            case 'p': port = atoi(optarg);  flags++; break;
            case 'd': directory = optarg;   flags++; break;
            case 's': demonise_process = false;      break;
            default : print_help_message(); exit(1);
        }
    }

    if (flags != 3){
        print_help_message();
        exit(1);
    }

    if (demonise_process && fork() != 0){
        return 0;
    }

    MasterServer server(1, ip, port, directory);
    server.run();
    return 0;
}