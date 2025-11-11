#include "../include/config.h"
#include <iostream>
#include <cstring>
#include <cstdlib>

ServerConfig ServerConfig::parse_args(int argc, char* argv[]) {
    ServerConfig config;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            print_help();
            exit(0);
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            config.client_db_file = argv[++i];
        } else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
            config.log_file = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            config.port = std::stoi(argv[++i]);
        }
    }
    
    return config;
}

void ServerConfig::print_help() {
    std::cout << "Usage: server [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h              Show this help message" << std::endl;
    std::cout << "  -d <file>       Client database file (default: /etc/vealc.conf)" << std::endl;
    std::cout << "  -l <file>       Log file (default: /var/log/vealc.log)" << std::endl;
    std::cout << "  -p <port>       Port number (default: 33333)" << std::endl;
}
