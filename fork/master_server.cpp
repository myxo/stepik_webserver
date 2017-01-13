#include "master_server.h"
#include "slave_server.h"
#include "fd_passing.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>

MasterServer::MasterServer(int process_number, std::string ip, int port, std::string folder)
    : process_num_(process_number)
    , master_port_(port)
    , ip_ (ip)
    , folder_(folder)
{
    slave_working_.resize(process_num_, false);
    for (int i = 0; i < process_number; i++){
        int sv[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1){
            std::cerr << "ERROR: socketpair return -1, on iteration " << i << std::endl;
        }
        int pid = fork();
        if (pid == 0){
            close(sv[0]);
            SlaveServer::run(sv[1], folder_);
            return;
        } else if (pid != -1){
            pairsocket_descriptors_.push_back(sv[0]);
            close(sv[1]);
            child_pid_.push_back(pid);
            std::cout << "Fork slave server. Pid - " << pid << std::endl;
        } else {
            std::cerr << "ERROR: fork return -1\n";
        }
    }
}

MasterServer::~MasterServer() {
    // тут нужно будет закрыть все дескрипторы
    close(master_descriptor_);
    kill_all_children();
}

void MasterServer::run(){
    master_descriptor_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (master_descriptor_ == -1){
        std::cerr << "Cannot create socket\n";
        kill_all_children();
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
    while(1){
        struct sockaddr_in client;
        int c = sizeof(struct sockaddr_in);
        int child_sock = accept(master_descriptor_, (struct sockaddr*)&client, (socklen_t*)&c);
        if (child_sock == -1){
            perror("accept fail: ");
            continue;
        }

        handle_connection(child_sock);

        //std::thread(connectoin_handler, child_sock, client).detach();
        
    }
}

void MasterServer::kill_all_children(){
    for (auto p: child_pid_){
        kill(p, SIGTERM);
    }
}

int MasterServer::find_free_slave(){
    // заменить алгоритмом
    for (size_t i = 0; i < slave_working_.size(); i++){
        if (!slave_working_[i]){
            return i;
        }
    }
    return -1;
}

void MasterServer::handle_connection(int socket_descriptor){
    int resting_slave = find_free_slave();

    if (resting_slave != -1){
        sock_fd_write(pairsocket_descriptors_[resting_slave], (void *)"1", 1, socket_descriptor);
    } else {
        work_queue_.push_back(socket_descriptor);
    }

}

void MasterServer::update_worker(){}