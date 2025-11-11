#include "../include/config.h"
#include "../include/server.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    try {
        ServerConfig config = ServerConfig::parse_args(argc, argv);
        Server server(config);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
