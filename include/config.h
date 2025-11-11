#ifndef CONFIG_H
#define CONFIG_H

#include <string>

struct ServerConfig {
    std::string client_db_file = "/etc/vealc.conf";
    std::string log_file = "/var/log/vealc.log";
    int port = 33333;
    
    static ServerConfig parse_args(int argc, char* argv[]);
    static void print_help();
};

#endif
