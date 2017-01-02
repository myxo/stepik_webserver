#include <iostream>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <thread>

const size_t MESSAGE_SIZE = 1024;

std::string parse_header(std::string message){
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
        std::cerr << "Unsupported method\n";
        return "";
    }
    // if (protocol_version != "HTTP/1.0"){
    //     std::cerr << "Unsupported protocol version\n";
    //     return "";
    // }

    path = path.substr(1);
    index1 = path.find('?');
    if (index1 != std::string::npos)
        path = path.substr(0,index1);
    return path;
}

void log(std::string& message){
    FILE *f = fopen("log", "w+");
    fprintf(f, "%s", message.c_str());
    fclose(f);
}

void log(char *message){
    FILE *f = fopen("log", "w+");
    fprintf(f, "%s", message);
    fclose(f);
}

std::string get_response(std::string path){
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

        response.append("HTTP/1.0 200 OK\r\n");
        response.append("Content-length: ");
        response.append(std::to_string(file_content.size()));
        response.append("\r\n");
        response.append("Content-Type: text/html\r\n\r\n");
        response.append(file_content);
    }

    return response;
}


void connectoin_handler(int socket, struct sockaddr_in client){
    char buffer[MESSAGE_SIZE] = {0};
    char *client_ip = inet_ntoa(client.sin_addr);
    int client_port = ntohs(client.sin_port);
    std::cout << "Connect from " << client_ip << ":" << client_port << std::endl;
    //char message[] = "Ok\n";
    //send(socket, message, strlen(message), MSG_NOSIGNAL);
    ssize_t recv_size = recv(socket, buffer, MESSAGE_SIZE, 0);
    std::cout << "Recieve message: " << buffer << std::endl << std::endl;
    log(buffer);
    
    std::string path = parse_header(buffer);
    if (path == "/"){
        std::cout << "root path\n";
        path = "index.html";
    }
    std::cout << "Getting " << path << std::endl;
    
    std::string response = get_response(path);

    std::cout << "-----\n";
    std::cout << response << std::endl;
    std::cout << "-----\n";
    send(socket, response.c_str(), response.size()+1, MSG_NOSIGNAL);
        
    shutdown(socket, SHUT_RDWR);
    close(socket);
}

void server_routine(std::string& ip, int port, std::string& directory){
    int descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (descriptor == -1){
        std::cerr << "Cannot create socket\n";
        exit(1);
    }
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(descriptor, (sockaddr *)(&sa), sizeof(sa)) < 0){
        // std::cerr << "Bind fail\n";
        perror("bind fail: ");
        exit(1);
    }
    if (listen(descriptor, SOMAXCONN) == -1){
        // std::cerr << "Listen fail: " << errno();
        perror("listen fail: ");
        exit(1);
    }

    char buffer[MESSAGE_SIZE];
    while(1){
        struct sockaddr_in client;
        int c = sizeof(struct sockaddr_in);
        int child_sock = accept(descriptor, (struct sockaddr*)&client, (socklen_t*)&c);
        if (child_sock == -1){
            perror("accept fail: ");
            continue;
        }

        std::thread(connectoin_handler, child_sock, client).detach();
        
    }

    close(descriptor);
}

void print_help_message(){
    std::cout << "usage: ./final -h <ip> -p <port> -d <directory>\n";
}

int main(int argc, char *argv[]){
    if (fork() != 0){
        return 0;
    }
    int opt, flags = 0, port;
    std::string ip, directory;

    while((opt = getopt(argc, argv, "h:p:d:")) != -1){
        switch(opt){
            case 'h': ip = optarg;          flags++; break;
            case 'p': port = atoi(optarg);  flags++; break;
            case 'd': directory = optarg;   flags++; break;
            default : print_help_message(); exit(1);
        }
    }

    if (flags != 3){
        print_help_message();
        exit(1);
    }

    server_routine(ip, port, directory);
    return 0;
}