#pragma once

#include <sys/types.h>
#include <sys/socket.h>

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <mutex>

class SlaveServerHandler{
public:
    SlaveServerHandler(int pid, int socket);
    ~SlaveServerHandler();

    int getId();
    int getPid();
    void send_work(int sending_socket);
    bool isWorking();
    bool checkIsDone();
    
private:
    int id_, pid_, message_socket_;
    bool working_;
    static int id_counter_;
};


class ServerPool{
public:
    ServerPool(int serve_num, std::string folder);
    ~ServerPool();
    void run_update_loop();
    bool add_work(int socket, bool add_to_queue = true);
private:
    void kill_all_children();
    int find_free_slave();
    void update_loop();
private:
    bool stop_update_loop = false;
    int server_num_;
    std::mutex lock;
    std::vector<std::shared_ptr<SlaveServerHandler>> slave_array_;
    std::vector<int> work_queue_;
};



