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
    
    // Приватные методы
    bool verify_authentication(const std::string& login, const std::string& salt, const std::string& hash);
    void process_vectors();
    uint32_t receive_uint32();
    std::vector<int32_t> receive_vector(uint32_t size);
    void send_uint32(uint32_t value);
    void send_int32(int32_t value);
    
    // Вспомогательные методы
    std::string calculate_md5(const std::string& data);
    std::string bytes_to_hex(const unsigned char* data, size_t length);

public:
    // Конструктор и основной метод
    Session(int client_socket, std::unordered_map<std::string, std::string>& clients, Logger& logger);
    void handle();
    
    // Методы для работы с текстом
    std::string receive_text_until_newline();
    std::string receive_text_exact(size_t length);
    bool send_text(const std::string& text);
};

#endif // SESSION_H
