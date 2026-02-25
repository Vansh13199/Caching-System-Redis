#include <iostream>
#include <csignal>
#include "server.h"
#include "storage.h"

Server* global_server = nullptr;

void signal_handler(int signal) {
    if (global_server) {
        std::cout << "\nStopping server..." << std::endl;
        global_server->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    int port = 6379; // Default Redis port
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    // Handle Ctrl+C for graceful shutdown
    std::signal(SIGINT, signal_handler);

    Storage storage;
    Server server(port, storage);

    global_server = &server;

    std::cout << "Starting Redis Clone on port " << port << "..." << std::endl;
    server.start();

    return 0;
}
