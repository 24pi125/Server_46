#include "../include/config.h"
#include <UnitTest++/UnitTest++.h>
#include <iostream>
#include <cstring>
#include <vector>

SUITE(ConfigTest) {
    TEST(DefaultValues) {
        char* argv[] = {(char*)"program", nullptr};
        ServerConfig config = ServerConfig::parse_args(1, argv);
        
        CHECK_EQUAL("/etc/vealc.conf", config.client_db_file);
        CHECK_EQUAL("/var/log/vealc.log", config.log_file);
        CHECK_EQUAL(33333, config.port);
    }
    
    TEST(CustomPort) {
        char* argv[] = {(char*)"program", (char*)"-p", (char*)"44444", nullptr};
        ServerConfig config = ServerConfig::parse_args(3, argv);
        
        CHECK_EQUAL(44444, config.port);
    }
    
    TEST(CustomConfigFile) {
        char* argv[] = {(char*)"program", (char*)"-c", (char*)"./myconfig.conf", nullptr};
        ServerConfig config = ServerConfig::parse_args(3, argv);
        
        CHECK_EQUAL("./myconfig.conf", config.client_db_file);
    }
    
    TEST(CustomLogFile) {
        char* argv[] = {(char*)"program", (char*)"-l", (char*)"./custom.log", nullptr};
        ServerConfig config = ServerConfig::parse_args(3, argv);
        
        CHECK_EQUAL("./custom.log", config.log_file);
    }
    
    TEST(MultipleOptions) {
        char* argv[] = {
            (char*)"program", 
            (char*)"-p", (char*)"55555",
            (char*)"-c", (char*)"./config.conf",
            (char*)"-l", (char*)"./server.log",
            nullptr
        };
        ServerConfig config = ServerConfig::parse_args(7, argv);
        
        CHECK_EQUAL(55555, config.port);
        CHECK_EQUAL("./config.conf", config.client_db_file);
        CHECK_EQUAL("./server.log", config.log_file);
    }
}

int main() {
    return UnitTest::RunAllTests();
}
