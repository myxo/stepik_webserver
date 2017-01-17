#include "master_server.h"
#include "slave_server.h"
#include "fd_passing.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <thread>
#include <iostream>

MasterServer::MasterServer(int process_number, std::string ip, int port, std::string folder)
    : process_num_(process_number)
    , master_port_(port)
    , ip_ (ip)
    , folder_(folder)
    , server_pool_(process_num_, folder)
    , stop_server_(false)
{
}

MasterServer::~MasterServer() {
    // тут нужно будет закрыть все дескрипторы
    close(master_descriptor_);
}

void MasterServer::run(){
    master_descriptor_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (master_descriptor_ == -1){
        std::cerr << "Cannot create socket\n";
        return;
    }
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(master_port_);
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(master_descriptor_, (sockaddr *)(&sa), sizeof(sa)) < 0){
        perror("bind fail: ");
        return;
    }
    if (listen(master_descriptor_, SOMAXCONN) == -1){
        perror("listen fail: ");
        return;
    }

    // char buffer[MESSAGE_SIZE];
    sleep(1);
    server_pool_.run_update_loop();
    while(!stop_server_){
        struct sockaddr_in client;
        int c = sizeof(struct sockaddr_in);
        int child_sock = accept(master_descriptor_, (struct sockaddr*)&client, (socklen_t*)&c);
        if (child_sock == -1){
            perror("accept fail: ");
            continue;
        }

        handle_connection(child_sock);
    }
}

void MasterServer::handle_connection(int socket_descriptor){
    server_pool_.add_work(socket_descriptor);
}

void MasterServer::stop_server(){
    stop_server_ = true;
}