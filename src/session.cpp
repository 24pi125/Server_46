#include "../include/session.h"
#include "logger.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

// Конструктор
Session::Session(int client_socket, std::unordered_map<std::string, std::string>& clients, Logger& logger)
    : client_socket(client_socket), clients(clients), logger(logger) {
}

// Основной метод обработки сессии
void Session::handle() {
    try {
        process_vectors();
    } catch (const std::exception& e) {
        logger.log("Session error: " + std::string(e.what()));
    }
    close(client_socket);
}

// Метод 1: Получить текст до новой строки
std::string Session::receive_text_until_newline() {
    std::string result;
    char buffer[1];
    
    while (true) {
        ssize_t bytes_received = recv(client_socket, buffer, 1, 0);
        if (bytes_received <= 0) {
            break;
        }
        if (buffer[0] == '\n') {
            break;
        }
        result += buffer[0];
    }
    
    // Убираем \r если есть
    if (!result.empty() && result.back() == '\r') {
        result.pop_back();
    }
    
    return result;
}

// Метод 2: Получить точное количество символов
std::string Session::receive_text_exact(size_t length) {
    std::string result;
    result.resize(length);
    char* buffer = &result[0];
    size_t total_received = 0;
    
    while (total_received < length) {
        ssize_t bytes_received = recv(client_socket, buffer + total_received, length - total_received, 0);
        if (bytes_received <= 0) {
            throw std::runtime_error("Connection closed or error");
        }
        total_received += bytes_received;
    }
    
    return result;
}

// Метод 3: Отправить текст
bool Session::send_text(const std::string& text) {
    size_t total_sent = 0;
    const char* buffer = text.c_str();
    size_t length = text.length();
    
    while (total_sent < length) {
        ssize_t bytes_sent = send(client_socket, buffer + total_sent, length - total_sent, 0);
        if (bytes_sent <= 0) {
            return false;
        }
        total_sent += bytes_sent;
    }
    
    return true;
}

// Вспомогательная функция: байты в hex строку
std::string Session::bytes_to_hex(const unsigned char* data, size_t length) {
    std::stringstream ss;
    for (size_t i = 0; i < length; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') 
           << static_cast<int>(data[i]);
    }
    return ss.str();
}

// Вспомогательная функция: вычисление MD5
std::string Session::calculate_md5(const std::string& data) {
    unsigned char md5_hash[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(data.c_str()), 
        data.length(), md5_hash);
    return bytes_to_hex(md5_hash, MD5_DIGEST_LENGTH);
}

// Проверка аутентификации (с солью от клиента)
bool Session::verify_authentication(const std::string& login, 
                                   const std::string& salt, 
                                   const std::string& received_hash) {
    // 1. Ищем пользователя в базе (из /etc/vealc.conf)
    auto it = clients.find(login);
    if (it == clients.end()) {
        logger.log("ERROR: User '" + login + "' not found in database");
        // Логируем какие пользователи есть в базе
        std::string available_users = "Available users: ";
        for (const auto& client : clients) {
            available_users += client.first + " ";
        }
        logger.log(available_users);
        return false;
    }
    
    std::string stored_password = it->second;
    
    // 2. Логируем для отладки
    logger.log("=== AUTHENTICATION VERIFICATION ===");
    logger.log("Login: " + login);
    logger.log("Salt (from client): " + salt);
    logger.log("Hash (from client): " + received_hash);
    logger.log("Password (from /etc/vealc.conf): " + stored_password);
    
    // 3. Проверяем длину соли (16 hex символов)
    if (salt.length() != 16) {
        logger.log("ERROR: Salt must be 16 hex characters, got " + std::to_string(salt.length()));
        return false;
    }
    
    // 4. Проверяем что соль состоит из hex символов
    for (char c : salt) {
        if (!isxdigit(c)) {
            logger.log("ERROR: Salt contains non-hex character: " + std::string(1, c));
            return false;
        }
    }
    
    // 5. Проверяем длину хэша (32 hex символа для MD5)
    if (received_hash.length() != 32) {
        logger.log("ERROR: Hash must be 32 hex characters for MD5, got " + std::to_string(received_hash.length()));
        return false;
    }
    
    // 6. Проверяем что хэш состоит из hex символов
    for (char c : received_hash) {
        if (!isxdigit(c)) {
            logger.log("ERROR: Hash contains non-hex character: " + std::string(1, c));
            return false;
        }
    }
    
    // 7. Вычисляем ожидаемый хэш: MD5(пароль + соль)
    std::string password_plus_salt = stored_password + salt;
    std::string expected_hash = calculate_md5(password_plus_salt);
    
    // Приводим к нижнему регистру для сравнения
    std::string received_lower = received_hash;
    std::transform(received_lower.begin(), received_lower.end(), received_lower.begin(), ::tolower);
    
    std::string expected_lower = expected_hash;
    std::transform(expected_lower.begin(), expected_lower.end(), expected_lower.begin(), ::tolower);
    
    logger.log("Calculated MD5(password + salt): " + expected_lower);
    logger.log("Received hash (from client): " + received_lower);
    
    // 8. Сравниваем
    if (received_lower != expected_lower) {
        logger.log("ERROR: Hash mismatch! Authentication failed.");
        return false;
    }
    
    logger.log("SUCCESS: Hash match! Authentication successful.");
    logger.log("=== END AUTHENTICATION ===");
    return true;
}

// Получение uint32 из сети
uint32_t Session::receive_uint32() {
    uint32_t value;
    std::string data = receive_text_exact(4);
    memcpy(&value, data.c_str(), 4);
    return ntohl(value);
}

// Получение вектора int32
std::vector<int32_t> Session::receive_vector(uint32_t size) {
    std::vector<int32_t> result(size);
    if (size == 0) return result;
    
    std::string data = receive_text_exact(size * 4);
    
    for (uint32_t i = 0; i < size; i++) {
        uint32_t temp;
        memcpy(&temp, data.c_str() + i * 4, 4);
        result[i] = static_cast<int32_t>(ntohl(temp));
    }
    
    return result;
}

// Отправка uint32
void Session::send_uint32(uint32_t value) {
    uint32_t network_value = htonl(value);
    send_text(std::string(reinterpret_cast<char*>(&network_value), 4));
}

// Отправка int32
void Session::send_int32(int32_t value) {
    uint32_t network_value = htonl(static_cast<uint32_t>(value));
    send_text(std::string(reinterpret_cast<char*>(&network_value), 4));
}

// Основная логика обработки векторов
void Session::process_vectors() {
    logger.log("=== CLIENT CONNECTED ===");
    
    // 1. Получаем строку аутентификации (клиент отправляет одну строку)
    std::string auth_line = receive_text_until_newline();
    
    if (auth_line.empty()) {
        logger.log("ERROR: Empty authentication line received");
        send_text("ERROR\n");
        return;
    }
    
    logger.log("Authentication line received (" + 
               std::to_string(auth_line.length()) + " chars): " + auth_line);
    
    // 2. Проверяем минимальную длину
    if (auth_line.length() < 52) { // "user"(4) + salt(16) + hash(32)
        logger.log("ERROR: Authentication line too short. Expected >=52, got " + 
                   std::to_string(auth_line.length()));
        send_text("ERROR\n");
        return;
    }
    
    // 3. Парсим строку
    // Формат: первые 4 символа = "user", потом 16 hex = соль, потом 32 hex = хэш
    std::string login = auth_line.substr(0, 4);
    std::string client_salt = auth_line.substr(4, 16);
    std::string client_hash = auth_line.substr(20, 32);
    
    // Выводим раздельно для преподавателя
    logger.log("=== PARSED CREDENTIALS ===");
    logger.log("Login: [" + login + "]");
    logger.log("Salt: [" + client_salt + "]");
    logger.log("Hash: [" + client_hash + "]");
    logger.log("=== VERIFICATION START ===");
    
    // 4. Проверяем аутентификацию
    if (!verify_authentication(login, client_salt, client_hash)) {
        logger.log("ERROR: Authentication FAILED");
        send_text("ERROR\n");
        return;
    }
    
    // 5. Отправляем подтверждение
    logger.log("SUCCESS: Authentication OK, sending OK to client");
    send_text("OK\n");
    
    // 6. Получаем количество векторов
    uint32_t vector_count = receive_uint32();
    logger.log("Vector count received: " + std::to_string(vector_count));
    
    // 7. Обрабатываем каждый вектор
    for (uint32_t i = 0; i < vector_count; i++) {
        logger.log("--- Processing Vector " + std::to_string(i + 1) + " ---");
        
        // Получаем размер вектора
        uint32_t vector_size = receive_uint32();
        logger.log("Vector size: " + std::to_string(vector_size));
        
        // Получаем сам вектор
        std::vector<int32_t> vector_data = receive_vector(vector_size);
        
        // Логируем значения
        if (!vector_data.empty()) {
            std::string values_str = "Values: ";
            for (size_t j = 0; j < vector_data.size() && j < 5; j++) {
                values_str += std::to_string(vector_data[j]);
                if (j < vector_data.size() - 1 && j < 4) values_str += ", ";
            }
            if (vector_data.size() > 5) values_str += " ...";
            logger.log(values_str + " (total " + std::to_string(vector_data.size()) + " values)");
        }
        
        // Вычисляем сумму элементов
        int64_t sum = 0;
        for (int32_t val : vector_data) {
            sum += val;
        }
        
        logger.log("Raw sum: " + std::to_string(sum));
        
        // Проверка на переполнение
        int32_t result;
        if (sum > INT32_MAX) {
            result = INT32_MAX;
            logger.log("Warning: Overflow, clipping to INT32_MAX");
        } else if (sum < INT32_MIN) {
            result = INT32_MIN;
            logger.log("Warning: Underflow, clipping to INT32_MIN");
        } else {
            result = static_cast<int32_t>(sum);
        }
        
        logger.log("Final result: " + std::to_string(result));
        
        // Отправляем результат
        send_int32(result);
        logger.log("Result sent successfully");
    }
    
    logger.log("=== SESSION COMPLETED SUCCESSFULLY ===");
    logger.log("Total vectors processed: " + std::to_string(vector_count));
    logger.log("======================================");
}
