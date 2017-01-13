#include "slave_server.h"
#include "fd_passing.h"
#include "log_utils.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <string>

SlaveServer::SlaveServer(int master_socket_descriptor, std::string root_folder)
     : master_socket_descriptor_(master_socket_descriptor)
     , root_folder_(root_folder)
{
    log("Slave create");
}

SlaveServer::~SlaveServer() {}

void SlaveServer::run(){
    log("slave run");
    while (true){
        char buffer[256];
        int listen_socket;
        sock_fd_read(master_socket_descriptor_, (void *)buffer, 256, &listen_socket);
        log(std::string("slave recieve socket ") + std::to_string(listen_socket));

        connectoin_handler(listen_socket);
    }
}


void SlaveServer::connectoin_handler(int socket){
    char buffer[MESSAGE_SIZE] = {0};
    recv(socket, buffer, MESSAGE_SIZE, 0);
    // std::cerr << "Recieve message: " << buffer << std::endl << std::endl;
    // log(buffer);
    
    std::string path = parse_header(buffer);
    if (path == "/"){
        // std::cerr << "root path\n";
        path = "index.html";
    }
    // std::cerr << "Getting " << path << std::endl;
    
    std::string response = get_response(root_folder_ + "/" + path);

    // std::cerr << "-----\n";
    // std::cerr << response << std::endl;
    // std::cerr << "-----\n";
    sleep(2);
    send(socket, response.c_str(), response.size()+1, MSG_NOSIGNAL);
        
    shutdown(socket, SHUT_RDWR);
    close(socket);
}

std::string SlaveServer::parse_header(std::string message){
    std::string method, path, protocol_version;

    int index1 = 0, index2 = message.find(' ');
    method = message.substr(index1, index2 - index1);
    index1 = index2 + 1;

    index2 = message.find(' ', index1);
    path = message.substr(index1, index2 - index1);
    index1 = index2 + 1;

    index2 = message.find(' ', index1);
    protocol_version = message.substr(index1, index2 - index1);
    index1 = index2 + 1;

    // if (protocol_version[protocol_version.size()-1] == '\n'){
    //     protocol_version.pop_back();
    // }

    if (method != "GET"){
        // std::cerr << "Unsupported method\n";
        return "";
    }
    // if (protocol_version != "HTTP/1.0"){
    //     std::cerr << "Unsupported protocol version\n";
    //     return "";
    // }

    path = path.substr(1);
    index1 = path.find('?');
    if (index1 != (int)std::string::npos)
        path = path.substr(0,index1);
    return path;
}


std::string SlaveServer::get_response(std::string path){
    std::string response;
    
    FILE *f = fopen(path.c_str(), "r");
    if (f == NULL){
        response.append("HTTP/1.0 404 NOT FOUND\r\n");
        response.append("Content-length: 0\r\n");
        response.append("Content-Type: text/html\r\n\r\n");
    } else {
        std::string file_content;

        while(1){
            char buffer[1024] = {0};
            int buff_size = fread(buffer, 1, 1024, f);
            if ( buff_size <= 0)
                break;
            file_content.append(buffer);
        }
        fclose(f);

        response.append("HTTP/1.0 200 OK\r\n");
        response.append("Content-length: ");
        response.append(std::to_string(file_content.size()));
        response.append("\r\n");
        response.append("Content-Type: text/html\r\n\r\n");
        response.append(file_content);
    }

    return response;
}

void SlaveServer::send_responce_to_master(){
    char message[] = "done";
    send(master_socket_descriptor_, message, sizeof(message), MSG_NOSIGNAL);
}