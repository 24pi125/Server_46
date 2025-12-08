#ifndef SESSION_H
#define SESSION_H

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

class Logger;

class Session {
private:
    int client_socket;
    std::unordered_map<std::string, std::string>& clients;
    Logger& logger;
    
    // Буфер для приема данных
    std::string receive_buffer;
    
    // Приватные методы
    void receive_to_buffer();
    std::string extract_from_buffer_exact(size_t length);
    std::string extract_hex_from_buffer(size_t length);  // ДОБАВЬТЕ ЭТУ СТРОКУ
    
    bool verify_authentication(const std::string& login, 
                              const std::string& salt, 
                              const std::string& received_hash);
    void process_vectors();
    uint32_t receive_uint32();
    std::vector<int32_t> receive_vector(uint32_t size);
    void send_uint32(uint32_t value);
    void send_int32(int32_t value);
    std::string calculate_md5(const std::string& data);
    int32_t calculate_vector_product(const std::vector<int32_t>& vector);

public:
    Session(int client_socket, std::unordered_map<std::string, std::string>& clients, Logger& logger);
    void handle();
    bool send_text(const std::string& text);
};

#endif // SESSION_H
