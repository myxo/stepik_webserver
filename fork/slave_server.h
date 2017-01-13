#pragma once

#include <string>

class SlaveServer{
public:
    static void run(int master_socket_descriptor, std::string root_folder) {
        SlaveServer server(master_socket_descriptor, root_folder);
        server.run();
    }
    ~SlaveServer();
private:
    SlaveServer(int master_socket_descriptor, std::string root_folder);
    void run();

    void connectoin_handler(int socket);
    std::string parse_header(std::string message);
    std::string get_response(std::string path);
    void send_responce_to_master();
private:
    int master_socket_descriptor_;
    std::string root_folder_;
    static const int MESSAGE_SIZE = 1024;
};