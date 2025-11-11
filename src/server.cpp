#include <iostream>
#include "../include/server.h"
#include "../include/session.h"
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>

Server::Server(const ServerConfig& config) 
    : config_(config), logger_(config.log_file), server_fd_(-1) {
    load_clients();
    setup_socket();
}

Server::~Server() {
    if (server_fd_ > 0) {
        close(server_fd_);
    }
}

void Server::load_clients() {
    std::ifstream file(config_.client_db_file);
    if (!file.is_open()) {
        logger_.log_error("Cannot open client database: " + config_.client_db_file, true);
        throw std::runtime_error("Cannot open client database");
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string login = line.substr(0, pos);
            std::string password = line.substr(pos + 1);
            clients_[login] = password;
        }
    }
    file.close();
    
    logger_.log("Loaded " + std::to_string(clients_.size()) + " clients");
}

void Server::setup_socket() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        logger_.log_error("Socket creation failed", true);
        throw std::runtime_error("Socket creation failed");
    }
    
    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        logger_.log_error("Setsockopt failed", true);
        throw std::runtime_error("Setsockopt failed");
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(config_.port);
    
    if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        logger_.log_error("Bind failed on port " + std::to_string(config_.port), true);
        throw std::runtime_error("Bind failed");
    }
    
    if (listen(server_fd_, 10) < 0) {
        logger_.log_error("Listen failed", true);
        throw std::runtime_error("Listen failed");
    }
    
    logger_.log("Server socket setup complete on port " + std::to_string(config_.port));
}

void Server::accept_connections() {
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    
    logger_.log("Server started, waiting for connections...");
    
    while (true) {
        int client_socket = accept(server_fd_, (struct sockaddr*)&address, &addrlen);
        if (client_socket < 0) {
            logger_.log_error("Accept failed", false);
            continue;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
        logger_.log("New connection from " + std::string(client_ip));
        
        Session session(client_socket, clients_, logger_);
        session.handle();
    }
}

void Server::run() {
    logger_.log("Server starting...");
    std::cout << "Server running on port " << config_.port << std::endl;
    std::cout << "Client database: " << config_.client_db_file << std::endl;
    std::cout << "Log file: " << config_.log_file << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    
    accept_connections();
}
