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
#include <vector>
#include <climits>

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

// Прием данных в буфер
void Session::receive_to_buffer() {
    char buffer[4096];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        receive_buffer.append(buffer, bytes_received);
    } else if (bytes_received == 0) {
        throw std::runtime_error("Connection closed by client");
    } else {
        throw std::runtime_error("Receive error");
    }
}

// Извлечение точного количества байт из буфера
std::string Session::extract_from_buffer_exact(size_t length) {
    while (receive_buffer.size() < length) {
        receive_to_buffer();
    }
    
    std::string result = receive_buffer.substr(0, length);
    receive_buffer.erase(0, length);
    return result;
}

// Отправка текста
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

// Вычисление MD5
std::string Session::calculate_md5(const std::string& data) {
    unsigned char md5_hash[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(data.c_str()), 
        data.length(), md5_hash);
    
    std::stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') 
           << static_cast<int>(md5_hash[i]);
    }
    return ss.str();
}

// Вычисление произведения вектора
int32_t Session::calculate_vector_product(const std::vector<int32_t>& vector) {
    if (vector.empty()) {
        return 0;
    }
    
    int64_t product = 1;
    bool overflow_detected = false;
    
    for (int32_t val : vector) {
        // Проверка переполнения
        if (val != 0 && llabs(product) > INT64_MAX / llabs(val)) {
            overflow_detected = true;
            break;
        }
        product *= val;
    }
    
    if (overflow_detected) {
        // Определяем знак
        int sign = 1;
        for (int32_t val : vector) {
            if (val > 0) {
                sign = 1;
                break;
            } else if (val < 0) {
                sign = -1;
                break;
            }
        }
        
        if (sign > 0) return INT32_MAX;
        else return INT32_MIN;
    }
    
    // Проверка границ int32
    if (product > INT32_MAX) return INT32_MAX;
    if (product < INT32_MIN) return INT32_MIN;
    
    return static_cast<int32_t>(product);
}

// Проверка аутентификации
bool Session::verify_authentication(const std::string& login, 
                                   const std::string& salt, 
                                   const std::string& received_hash) {
    // Ищем пользователя в базе
    auto it = clients.find(login);
    if (it == clients.end()) {
        logger.log("ERROR: User '" + login + "' not found in database");
        return false;
    }
    
    std::string stored_password = it->second;
    
    // Логируем для отладки
    logger.log("=== AUTHENTICATION VERIFICATION ===");
    logger.log("Login: " + login);
    logger.log("Salt: " + salt);
    logger.log("Hash from client: " + received_hash);
    logger.log("Password from DB: " + stored_password);
    
    // Проверяем форматы
    if (salt.length() != 16) {
        logger.log("ERROR: Salt must be 16 hex chars");
        return false;
    }
    
    if (received_hash.length() != 32) {
        logger.log("ERROR: Hash must be 32 hex chars for MD5");
        return false;
    }
    
    // Правильный порядок - соль + пароль
    std::string salt_plus_password = salt + stored_password;
    logger.log("String for MD5 (salt+password): " + salt_plus_password);
    
    std::string expected_hash = calculate_md5(salt_plus_password);
    
    // Приводим к нижнему регистру
    std::string received_lower = received_hash;
    std::transform(received_lower.begin(), received_lower.end(), received_lower.begin(), ::tolower);
    
    std::string expected_lower = expected_hash;
    std::transform(expected_lower.begin(), expected_lower.end(), expected_lower.begin(), ::tolower);
    
    logger.log("Expected MD5: " + expected_lower);
    logger.log("Received MD5: " + received_lower);
    
    // Сравниваем
    bool success = (received_lower == expected_lower);
    
    if (success) {
        logger.log("SUCCESS: Authentication passed");
    } else {
        logger.log("ERROR: Authentication failed - hash mismatch");
    }
    
    return success;
}

// Получение uint32 (без ntohl)
uint32_t Session::receive_uint32() {
    std::string data = extract_from_buffer_exact(4);
    uint32_t value;
    memcpy(&value, data.c_str(), 4);
    return value;
}

// Получение вектора int32 (без ntohl)
std::vector<int32_t> Session::receive_vector(uint32_t size) {
    std::vector<int32_t> result(size);
    if (size == 0) return result;
    
    std::string data = extract_from_buffer_exact(size * 4);
    
    for (uint32_t i = 0; i < size; i++) {
        uint32_t temp;
        memcpy(&temp, data.c_str() + i * 4, 4);
        result[i] = static_cast<int32_t>(temp);
    }
    
    return result;
}

// Отправка uint32 (без htonl)
void Session::send_uint32(uint32_t value) {
    send_text(std::string(reinterpret_cast<char*>(&value), 4));
}

// Отправка int32 (без htonl)
void Session::send_int32(int32_t value) {
    send_text(std::string(reinterpret_cast<char*>(&value), 4));
}

// Основная логика обработки векторов
void Session::process_vectors() {
    logger.log("=== NEW CLIENT CONNECTION ===");
    
    try {
        // 1. Получаем аутентификационные данные
        // Клиент отправляет: "user" + 16hex соль + 32hex хэш = 52 символа
        
        // Ждем пока не получим достаточно данных
        while (receive_buffer.size() < 52) {
            receive_to_buffer();
        }
        
        // Логируем полученные данные
        std::string auth_data = receive_buffer.substr(0, 52);
        logger.log("Authentication data received (52 chars): " + auth_data);
        
        // 2. Парсим ФИКСИРОВАННОЙ длины
        // Позиции 0-3: "user" (всегда 4 символа)
        // Позиции 4-19: соль (16 hex символов)
        // Позиции 20-51: хэш (32 hex символа)
        
        std::string login = "user";  // Клиент всегда отправляет "user"
        std::string client_salt = receive_buffer.substr(4, 16);
        std::string client_hash = receive_buffer.substr(20, 32);
        
        // Удаляем обработанные данные
        receive_buffer.erase(0, 52);
        
        logger.log("=== PARSED CREDENTIALS ===");
        logger.log("Login: " + login);
        logger.log("Salt: " + client_salt);
        logger.log("Hash: " + client_hash);
        
        // 3. Проверяем аутентификацию
        if (!verify_authentication(login, client_salt, client_hash)) {
            logger.log("ERROR: Authentication FAILED");
            send_text("ERROR\n");
            return;
        }
        
        // 4. Отправляем подтверждение
        logger.log("SUCCESS: Authentication OK, sending OK to client");
        send_text("OK\n");
        
        // 5. Получаем количество векторов
        uint32_t vector_count = receive_uint32();
        logger.log("Vector count: " + std::to_string(vector_count));
        
        // 6. Обрабатываем каждый вектор
        for (uint32_t i = 0; i < vector_count; i++) {
            logger.log("--- Processing Vector " + std::to_string(i + 1) + " ---");
            
            // Получаем размер вектора
            uint32_t vector_size = receive_uint32();
            logger.log("Vector size: " + std::to_string(vector_size));
            
            // Получаем данные вектора
            std::vector<int32_t> vector_data = receive_vector(vector_size);
            
            // Логируем первые значения
            if (!vector_data.empty()) {
                std::string values = "Values: ";
                for (size_t j = 0; j < std::min((size_t)5, vector_data.size()); j++) {
                    values += std::to_string(vector_data[j]) + " ";
                }
                if (vector_data.size() > 5) values += "...";
                logger.log(values);
            }
            
            // Вычисляем ПРОИЗВЕДЕНИЕ элементов вектора
            int32_t product = calculate_vector_product(vector_data);
            logger.log("Product result: " + std::to_string(product));
            
            // Отправляем результат клиенту
            send_int32(product);
            logger.log("Result sent");
        }
        
        logger.log("=== SESSION COMPLETED SUCCESSFULLY ===");
        logger.log("Total vectors processed: " + std::to_string(vector_count));
        
    } catch (const std::exception& e) {
        logger.log("ERROR in process_vectors: " + std::string(e.what()));
    }
}
