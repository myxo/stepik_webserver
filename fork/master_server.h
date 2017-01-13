#pragma once

#include <sys/types.h>
#include <sys/socket.h>

#include <string>
#include <vector>
#include <utility>
#include <memory>

class MasterServer{
public:
    MasterServer(int process_number, std::string ip, int port, std::string folder);
    ~MasterServer();
    void run();

private:
    std::string parse_header(std::string message);
    std::string get_response(std::string path);

    // kill all children processes
    void kill_all_children();
    int find_free_slave();
    void handle_connection(int socket_descriptor);
    void update_worker();
    void pass_descriptor(int slave, int socket_descriptor);

private:
    std::vector<int> pairsocket_descriptors_;
    std::vector<int> child_pid_;
    const int process_num_;
    int master_port_;
    int master_descriptor_;
    std::string ip_, folder_;
    std::vector<bool> slave_working_;
    std::vector<int> work_queue_;
};