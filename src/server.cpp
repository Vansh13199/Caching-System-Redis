#include "server.h"
#include <iostream>
#include <sstream>

#ifdef _WIN32
// Windows socket setup
#else
// POSIX socket setup
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#endif

Server::Server(int port, Storage& storage) : port_(port), storage_(storage), running_(false) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        exit(1);
    }
#endif
}

Server::~Server() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

void Server::start() {
    running_ = true;

#ifdef _WIN32
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        return;
    }
#else
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "Failed to create socket." << std::endl;
        return;
    }
    
    // Set SO_REUSEADDR
    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "Failed to set socket options." << std::endl;
        return;
    }
#endif

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

#ifdef _WIN32
    if (bind(server_fd_, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(server_fd_);
        return;
    }
    if (listen(server_fd_, 10) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(server_fd_);
        return;
    }
#else
    if (bind(server_fd_, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed." << std::endl;
        return;
    }
    if (listen(server_fd_, 10) < 0) {
        std::cerr << "Listen failed." << std::endl;
        return;
    }
#endif

    std::cout << "Server listening on port " << port_ << std::endl;

    while (running_) {
        struct sockaddr_in client_address;
#ifdef _WIN32
        int addrlen = sizeof(client_address);
        SOCKET client_socket = accept(server_fd_, (struct sockaddr *)&client_address, &addrlen);
        if (client_socket == INVALID_SOCKET) {
            if (running_) std::cerr << "Accept failed." << std::endl;
            continue;
        }
#else
        socklen_t addrlen = sizeof(client_address);
        int client_socket = accept(server_fd_, (struct sockaddr *)&client_address, &addrlen);
        if (client_socket < 0) {
            if (running_) std::cerr << "Accept failed." << std::endl;
            continue;
        }
#endif
        
        // Handle new connection in a separate thread
        client_threads_.emplace_back(&Server::handle_client, this, client_socket);
    }
}

void Server::stop() {
    if (!running_) return;
    running_ = false;
    
#ifdef _WIN32
    closesocket(server_fd_);
#else
    close(server_fd_);
#endif

    for (auto& th : client_threads_) {
        if (th.joinable()) {
            th.join();
        }
    }
}

#ifdef _WIN32
void Server::handle_client(SOCKET client_socket) {
#else
void Server::handle_client(int client_socket) {
#endif
    char buffer[1024] = {0};
    
    while (running_) {
#ifdef _WIN32
        int valread = recv(client_socket, buffer, 1024, 0);
#else
        int valread = read(client_socket, buffer, 1024);
#endif
        if (valread <= 0) {
            // Client disconnected or error
            break;
        }

        std::string command_str(buffer, valread);
        
        // Remove trailing newlines/carriage returns
        while (!command_str.empty() && (command_str.back() == '\n' || command_str.back() == '\r')) {
            command_str.pop_back();
        }

        if (command_str.empty()) continue;

        std::string response = process_command(command_str) + "\r\n";
        
#ifdef _WIN32
        send(client_socket, response.c_str(), response.length(), 0);
#else
        send(client_socket, response.c_str(), response.length(), 0);
#endif
    }

#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
}

std::vector<std::string> Server::split_args(const std::string& str) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string Server::process_command(const std::string& command_str) {
    std::vector<std::string> args = split_args(command_str);
    if (args.empty()) return "-ERR empty command";

    std::string cmd = args[0];
    // Convert to upper case for case-insensitive comparison
    for (char& c : cmd) c = std::toupper(c);

    if (cmd == "PING") {
        return "+PONG";
    } 
    else if (cmd == "SET") {
        if (args.size() < 3) return "-ERR wrong number of arguments for 'set' command";
        // for simple implementation, we assume the rest of the string is the value
        // if there are multiple parts, join them or just take args[2] as per simple Redis rules
        std::string value = args[2];
        for (size_t i = 3; i < args.size(); ++i) {
            value += " " + args[i];
        }
        storage_.set(args[1], value);
        return "+OK";
    } 
    else if (cmd == "GET") {
        if (args.size() != 2) return "-ERR wrong number of arguments for 'get' command";
        auto val = storage_.get(args[1]);
        if (val.first) {
            // Bulk string reply
            return "$" + std::to_string(val.second.length()) + "\r\n" + val.second;
        } else {
            return "$-1"; // Null reply
        }
    } 
    else if (cmd == "DEL") {
        if (args.size() < 2) return "-ERR wrong number of arguments for 'del' command";
        int count = 0;
        for (size_t i = 1; i < args.size(); ++i) {
            if (storage_.del(args[i])) count++;
        }
        return ":" + std::to_string(count); // Integer reply
    }

    return "-ERR unknown command '" + cmd + "'";
}
