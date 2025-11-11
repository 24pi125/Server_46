#ifndef SESSION_H
#define SESSION_H

#include "types.h"
#include "logger.h"
#include <unordered_map>
#include <string>
#include <vector>

class Session {
private:
    int client_socket_;
    std::unordered_map<std::string, std::string>& clients_;
    Logger& logger_;
    
    bool authenticate();
    bool receive_login(std::string& login);
    bool send_salt(const std::string& salt);
    bool receive_hash(std::string& hash);
    bool verify_authentication(const std::string& login, const std::string& hash, const std::string& salt);
    void process_vectors();
    uint32_t receive_uint32();
    Vector receive_vector(uint32_t size);
    void send_results(const std::vector<int32_t>& results);
    void send_uint32(uint32_t value);
    void send_int32(int32_t value);
    
public:
    Session(int client_socket, std::unordered_map<std::string, std::string>& clients, Logger& logger);
    void handle();
};

#endif
