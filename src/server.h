#ifndef REDIS_CLONE_SERVER_H
#define REDIS_CLONE_SERVER_H

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include "storage.h"

// Platform-specific socket headers
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

class Server {
public:
    Server(int port, Storage& storage);
    ~Server();

    // Start listening and accepting connections
    void start();
    void stop();

private:
#ifdef _WIN32
    SOCKET server_fd_;
#else
    int server_fd_;
#endif
    int port_;
    Storage& storage_;
    std::atomic<bool> running_;
    std::vector<std::thread> client_threads_;

    // Handle a single client connection
#ifdef _WIN32
    void handle_client(SOCKET client_socket);
#else
    void handle_client(int client_socket);
#endif

    // Parse command and return response
    std::string process_command(const std::string& command_str);
    
    // Split string by spaces into arguments
    std::vector<std::string> split_args(const std::string& str);
};

#endif // REDIS_CLONE_SERVER_H
