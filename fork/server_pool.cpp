#include "server_pool.h"
#include "fd_passing.h"
#include "slave_server.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <thread>
#include <iostream>

int SlaveServerHandler::id_counter_ = 0;

SlaveServerHandler::SlaveServerHandler(int pid, int socket)
    : id_(id_counter_)
    , pid_(pid)
    , message_socket_(socket)
    , working_(false)
{
    id_counter_++;
}

SlaveServerHandler::~SlaveServerHandler(){}

int SlaveServerHandler::getId(){
    return id_;
}

int SlaveServerHandler::getPid(){
    return pid_;
}

void SlaveServerHandler::send_work(int sending_socket){
    sock_fd_write(message_socket_, (void *)"1", 1, sending_socket);
    working_ = true;
    close(sending_socket);
}

bool SlaveServerHandler::isWorking(){
    return working_;
}

bool SlaveServerHandler::checkIsDone(){
    char buffer[256];

    int size = recv(message_socket_, buffer, 256, MSG_DONTWAIT);
    if (size == -1){
        return false;
    }

    if (size > 0 && !strncmp(buffer, "done", 4)){
        working_ = false;
        std::cout << "free slave =)\n";
        return true;
    }
    return false;
}


ServerPool::ServerPool(int serve_num, std::string folder)
    : server_num_(serve_num)
{
    for (int i = 0; i < server_num_; i++){
        int sv[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1){
            std::cerr << "ERROR: socketpair return -1, on iteration " << i << std::endl;
        }
        int pid = fork();
        if (pid == 0){
            close(sv[0]);
            SlaveServer::run(sv[1], folder);
            return; // TODO maybe exit(1)??? think about it.
        } else if (pid != -1){
            slave_array_.push_back(std::make_shared<SlaveServerHandler>(pid, sv[0]));
            close(sv[1]);
            std::cout << "Fork slave server. Pid - " << pid << " id - " << slave_array_[i]->getId() << std::endl;
        } else {
            std::cerr << "ERROR: fork return -1\n";
        }
    }
}

ServerPool::~ServerPool(){
    kill_all_children();
}

void ServerPool::run_update_loop(){
    std::thread(&ServerPool::update_loop, this).detach();
}

void ServerPool::update_loop(){
    while(1){
        // std::cout << "update loop\n";
        for (auto& x: slave_array_){
            x->checkIsDone();
        }
        for (size_t i = 0; i < work_queue_.size(); i++){
            bool add_result = add_work(work_queue_.back(), false);
            if (add_result == false){
                break;
            } else {
                work_queue_.pop_back();
            }
        }
        sleep(1); // TODO change to microsleep
    }
}

void ServerPool::kill_all_children(){
    for (auto p: slave_array_){
        kill(p->getPid(), SIGTERM);
    }
}

int ServerPool::find_free_slave(){
    // заменить алгоритмом
    for (size_t i = 0; i < slave_array_.size(); i++){
        if (!(slave_array_[i]->isWorking())){
            return i;
        }
    }
    return -1;
}

bool ServerPool::add_work(int socket, bool add_to_queue){
    int resting_slave = find_free_slave();

    if (resting_slave != -1){
        std::cout << "send connection to " << slave_array_[resting_slave]->getId() << " slave\n";
        slave_array_[resting_slave]->send_work(socket);
        return true;
    } else if (add_to_queue) {
        std::cout << "Stash connection\n";
        work_queue_.push_back(socket);
        return false;
    }
    return false;
}

