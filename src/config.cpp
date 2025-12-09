#include "../include/config.h"
#include <iostream>
#include <cstring>
#include <cstdlib>

ServerConfig ServerConfig::parse_args(int argc, char* argv[]) {
    ServerConfig config;
    
    // Если нет аргументов - возвращаем конфиг по умолчанию
    if (argc == 1) {
        return config;  // Просто возвращаем дефолтный конфиг
    }
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
            exit(0);
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            config.client_db_file = argv[++i];
        } else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
            config.log_file = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            config.port = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            config.client_db_file = argv[++i];
        } else {
            // Неизвестный аргумент
            std::cerr << "Unknown option: " << argv[i] << "\n\n";
            print_help();
            exit(1);
        }
    }
    
    return config;
}

void ServerConfig::print_help() {
    std::cout << "Usage: ./server [options]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help       Show this help message\n";
    std::cout << "  -p PORT          Server port (default: 33333)\n";
    std::cout << "  -c CONFIG_FILE   Client database file (default: /etc/vealc.conf)\n";
    std::cout << "  -d CONFIG_FILE   Alias for -c\n";
    std::cout << "  -l LOG_FILE      Log file (default: /var/log/vealc.log)\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  ./server                    # Run with default settings\n";
    std::cout << "  ./server -p 44444           # Run on port 44444\n";
    std::cout << "  ./server -c ./myconfig.conf # Use custom config file\n";
}
