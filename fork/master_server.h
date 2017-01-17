#pragma once

#include "server_pool.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <mutex>



class MasterServer{
public:
    MasterServer(int process_number, std::string ip, int port, std::string folder);
    ~MasterServer();
    void run();
    void stop_server();

private:
    void handle_connection(int socket_descriptor);
    
private:
    const int process_num_;
    int master_port_;
    int master_descriptor_;
    std::string ip_, folder_;
    ServerPool server_pool_;
    bool stop_server_;
};