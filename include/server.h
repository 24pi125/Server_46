#ifndef SERVER_H
#define SERVER_H

#include "types.h"
#include "config.h"
#include "logger.h"
#include <unordered_map>
#include <string>

class Server {
private:
    ServerConfig config_;
    Logger logger_;
    std::unordered_map<std::string, std::string> clients_;
    int server_fd_;
    
    void load_clients();
    void setup_socket();
    void accept_connections();
    
public:
    Server(const ServerConfig& config);
    ~Server();
    void run();
};

#endif
