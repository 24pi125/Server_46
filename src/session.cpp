/**
 * @file session.cpp
 * @brief Реализация обработки клиентских сессий
 * 
 * Содержит реализацию методов класса Session:
 * - обработка аутентификации по MD5
 * - прием и парсинг бинарных данных
 * - вычисление произведений векторов с контролем переполнения
 * - сетевое взаимодействие с клиентом
 * 
 * @see session.h
 */

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

/**
 * @brief Конструктор сессии
 * 
 * @param client_socket Сокет подключенного клиента
 * @param clients База данных клиентов
 * @param logger Логгер для записи событий
 */
Session::Session(int client_socket, std::unordered_map<std::string, std::string>& clients, Logger& logger)
    : client_socket(client_socket), clients(clients), logger(logger) {
}

/**
 * @brief Основной метод обработки сессии
 * 
 * Выполняет полный цикл обработки клиентского запроса:
 * 1. Аутентификация клиента
 * 2. Прием векторов данных
 * 3. Вычисление произведений
 * 4. Отправка результатов
 * 
 * @note Закрывает клиентский сокет при завершении (успешном или с ошибкой)
 * @throw std::exception перехватывает и логирует исключения
 */
void Session::handle() {
    try {
        process_vectors();
    } catch (const std::exception& e) {
        logger.log("Session error: " + std::string(e.what()));
    }
    close(client_socket);
}

/**
 * @brief Прием данных в буфер
 * 
 * Принимает данные из сокета и добавляет их во внутренний буфер.
 * Блокирующая операция - ждет поступления данных.
 * 
 * @throw std::runtime_error при закрытии соединения или ошибке приема
 */
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

/**
 * @brief Извлечение точного количества байт из буфера
 * 
 * @param length Количество байт для извлечения
 * @return std::string Извлеченные данные
 * 
 * @details
 * Если в буфере недостаточно данных, вызывает receive_to_buffer()
 * до тех пор, пока не наберется нужное количество байт.
 * 
 * @note Удаляет извлеченные данные из буфера
 */
std::string Session::extract_from_buffer_exact(size_t length) {
    while (receive_buffer.size() < length) {
        receive_to_buffer();
    }
    
    std::string result = receive_buffer.substr(0, length);
    receive_buffer.erase(0, length);
    return result;
}

/**
 * @brief Отправка текста клиенту
 * 
 * @param text Текст для отправки
 * @return bool true если отправка успешна, false при ошибке
 * 
 * @details
 * Отправляет данные частями, пока все не будет отправлено.
 * Обрабатывает частичную отправку (short write).
 */
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

/**
 * @brief Вычисление MD5 хэша
 * 
 * @param data Данные для хэширования
 * @return std::string MD5 хэш в hex формате (32 символа, lowercase)
 * 
 * @note Использует OpenSSL MD5 implementation
 */
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

/**
 * @brief Вычисление произведения элементов вектора
 * 
 * @param vector Вектор целых чисел
 * @return int32_t Произведение элементов с контролем переполнения
 * 
 * @details
 * Алгоритм:
 * 1. Использует 64-битные вычисления для предотвращения промежуточного переполнения
 * 2. Проверяет переполнение перед каждым умножением
 * 3. При переполнении возвращает INT32_MAX или INT32_MIN
 * 
 * @note Для пустого вектора возвращает 0
 */
int32_t Session::calculate_vector_product(const std::vector<int32_t>& vector) {
    if (vector.empty()) {
        return 0;
    }
    
    int64_t product = 1;
    for (int32_t val : vector) {
        // Проверка переполнения при умножении
        // Используем static_cast<int64_t> для безопасного преобразования
        int64_t val64 = static_cast<int64_t>(val);
        
        if (val64 != 0 && llabs(product) > INT64_MAX / llabs(val64)) {
            // Переполнение
            if ((product > 0 && val64 > 0) || (product < 0 && val64 < 0)) {
                return INT32_MAX;
            } else {
                return INT32_MIN;
            }
        }
        product *= val64;
    }
    
    // Проверка выхода за пределы int32
    if (product > INT32_MAX) {
        return INT32_MAX;
    } else if (product < INT32_MIN) {
        return INT32_MIN;
    }
    
    return static_cast<int32_t>(product);
}

/**
 * @brief Проверка аутентификации клиента
 * 
 * @param login Логин клиента
 * @param salt Соль (16 hex символов)
 * @param received_hash Хэш от клиента (32 hex символа)
 * @return bool true если аутентификация успешна
 * 
 * @details
 * Выполняет следующие проверки:
 * 1. Существование логина в базе
 * 2. Формат соли и хэша (длина и hex символы)
 * 3. Сравнение MD5(salt + stored_password) с полученным хэшем
 * 
 * @note Детально логирует все шаги проверки для отладки
 */
bool Session::verify_authentication(const std::string& login, 
                                   const std::string& salt, 
                                   const std::string& received_hash) {
    // Ищем пользователя в базе
    auto it = clients.find(login);
    if (it == clients.end()) {
        logger.log("err: User '" + login + "' not found");
        return false;
    }
    
    std::string stored_password = it->second;
    
    // Логируем для отладки
    logger.log("=== AUTHENTICATION ===");
    logger.log("Login: " + login);
    logger.log("Salt: " + salt);
    logger.log("Received hash: " + received_hash);
    logger.log("Stored password: " + stored_password);
    
    // Проверяем форматы
    if (salt.length() != 16) {
        logger.log("err: Salt must be 16 hex chars");
        return false;
    }
    
    if (received_hash.length() != 32) {
        logger.log("err: Hash must be 32 hex chars");
        return false;
    }
    
    // Проверяем что соль и хэш состоят из hex символов
    for (char c : salt) {
        if (!isxdigit(c)) {
            logger.log("err: Salt contains non-hex character");
            return false;
        }
    }
    
    for (char c : received_hash) {
        if (!isxdigit(c)) {
            logger.log("err: Hash contains non-hex character");
            return false;
        }
    }
    
    // ВЫЧИСЛЯЕМ MD5(СОЛЬ + ПАРОЛЬ)
    std::string salt_plus_password = salt + stored_password;
    std::string expected_hash = calculate_md5(salt_plus_password);
    
    logger.log("String for MD5 (salt+password): '" + salt_plus_password + "'");
    
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
        logger.log("err: Authentication failed - hash mismatch");
        logger.log("Check: 1) Password in /etc/vealc.conf is 'P@ssl@rd'");
        logger.log("       2) MD5 calculation uses 'salt + password' order");
    }
    
    return success;
}

/**
 * @brief Прием 32-битного беззнакового числа
 * 
 * @return uint32_t Принятое число
 * 
 * @note Данные ожидаются в сетевом порядке байт (big-endian)
 * @note Не использует ntohl() - предполагается что данные уже в правильном порядке
 */
uint32_t Session::receive_uint32() {
    std::string data = extract_from_buffer_exact(4);
    uint32_t value;
    memcpy(&value, data.c_str(), 4);
    return value; // Не используем ntohl - данные уже в правильном порядке
}

/**
 * @brief Прием вектора целых чисел
 * 
 * @param size Количество элементов в векторе
 * @return std::vector<int32_t> Принятый вектор
 * 
 * @note Каждый элемент - 4 байта в сетевом порядке байт
 */
std::vector<int32_t> Session::receive_vector(uint32_t size) {
    std::vector<int32_t> result(size);
    if (size == 0) return result;
    
    std::string data = extract_from_buffer_exact(size * 4);
    
    for (uint32_t i = 0; i < size; i++) {
        uint32_t temp;
        memcpy(&temp, data.c_str() + i * 4, 4);
        result[i] = static_cast<int32_t>(temp); // Не используем ntohl
    }
    
    return result;
}

/**
 * @brief Отправка 32-битного беззнакового числа
 * 
 * @param value Число для отправки
 * 
 * @note Отправляет в сетевом порядке байт
 * @note Не использует htonl() - предполагается что данные уже в правильном порядке
 */
void Session::send_uint32(uint32_t value) {
    send_text(std::string(reinterpret_cast<char*>(&value), 4));
}

/**
 * @brief Отправка 32-битного знакового числа
 * 
 * @param value Число для отправки
 */
void Session::send_int32(int32_t value) {
    send_text(std::string(reinterpret_cast<char*>(&value), 4));
}

/**
 * @brief Извлечение hex символов из буфера до не-hex символа
 * 
 * @return std::string Извлеченная последовательность hex символов
 * 
 * @note Используется для извлечения соли и хэша из потока данных
 */
std::string Session::extract_from_buffer_until_non_hex() {
    size_t i = 0;
    while (i < receive_buffer.size()) {
        char c = receive_buffer[i];
        if (!((c >= '0' && c <= '9') || 
              (c >= 'A' && c <= 'F') || 
              (c >= 'a' && c <= 'f'))) {
            break;
        }
        i++;
    }
    
    if (i == 0) {
        return "";
    }
    
    std::string result = receive_buffer.substr(0, i);
    receive_buffer.erase(0, i);
    return result;
}

/**
 * @brief Основная логика обработки векторов
 * 
 * Выполняет полный цикл обработки:
 * 1. Поиск и парсинг аутентификационных данных (48 hex символов)
 * 2. Проверка аутентификации
 * 3. Прием количества векторов
 * 4. Обработка каждого вектора
 * 5. Отправка результатов
 * 
 * @details
 * Формат входных данных:
 * [логин][16 hex соль][32 hex хэш][количество_векторов][вектор1]...[векторN]
 * 
 * @throw std::exception при ошибках парсинга или сетевого взаимодействия
 */
void Session::process_vectors() {
    logger.log("=== NEW CLIENT CONNECTION ===");
    
    try {
        // 1. Получаем данные
        receive_to_buffer();
        
        // Логируем сырые данные для отладки
        logger.log("Raw buffer (first 100 chars): " + 
                   receive_buffer.substr(0, std::min((size_t)100, receive_buffer.size())));
        
        // 2. Ищем 48 HEX СИМВОЛОВ ПОДРЯД (соль+хэш)
        // Соль (16 hex) + хэш (32 hex) = 48 hex символов
        
        size_t hex_start = 0;
        bool found = false;
        
        // Ищем в первых 100 символах (логин обычно короткий)
        size_t search_limit = std::min((size_t)100, receive_buffer.size());
        
        for (size_t i = 0; i < search_limit && !found; i++) {
            size_t hex_count = 0;
            size_t j = i;
            
            // Считаем сколько hex символов подряд начиная с позиции i
            while (j < receive_buffer.size() && hex_count < 48) {
                char c = receive_buffer[j];
                if ((c >= '0' && c <= '9') || 
                    (c >= 'A' && c <= 'F') || 
                    (c >= 'a' && c <= 'f')) {
                    hex_count++;
                    j++;
                } else {
                    break;
                }
            }
            
            // Если нашли 48 hex символов подряд
            if (hex_count == 48) {
                hex_start = i;
                found = true;
                logger.log("Found 48 hex chars starting at position: " + std::to_string(hex_start));
                break;
            }
        }
        
        if (!found) {
            // Получаем больше данных и пробуем снова
            receive_to_buffer();
            search_limit = std::min((size_t)150, receive_buffer.size());
            
            for (size_t i = 0; i < search_limit && !found; i++) {
                size_t hex_count = 0;
                size_t j = i;
                
                while (j < receive_buffer.size() && hex_count < 48) {
                    char c = receive_buffer[j];
                    if ((c >= '0' && c <= '9') || 
                        (c >= 'A' && c <= 'F') || 
                        (c >= 'a' && c <= 'f')) {
                        hex_count++;
                        j++;
                    } else {
                        break;
                    }
                }
                
                if (hex_count == 48) {
                    hex_start = i;
                    found = true;
                    logger.log("Found 48 hex chars (2nd attempt) at position: " + std::to_string(hex_start));
                    break;
                }
            }
        }
        
        if (!found) {
            logger.log("err: Cannot find 48 hex characters (salt+hash)");
            logger.log("Buffer size: " + std::to_string(receive_buffer.size()));
            send_text("err\n");
            return;
        }
        
        // 3. Извлекаем части
        // Все что до hex_start - это логин
        std::string login = receive_buffer.substr(0, hex_start);
        
        // Следующие 16 символов - соль
        std::string client_salt = receive_buffer.substr(hex_start, 16);
        
        // Следующие 32 символа - хэш
        std::string client_hash = receive_buffer.substr(hex_start + 16, 32);
        
        // Удаляем обработанные данные
        receive_buffer.erase(0, hex_start + 48);
        
        logger.log("=== PARSED CREDENTIALS ===");
        logger.log("Login: '" + login + "' (length: " + std::to_string(login.length()) + ")");
        logger.log("Salt: " + client_salt);
        logger.log("Hash: " + client_hash);
        
        // 4. Проверяем что логин не пустой
        if (login.empty()) {
            logger.log("err: Empty login");
            send_text("err\n");
            return;
        }
        
        // 5. Проверяем что соль и хэш действительно hex
        bool salt_valid = true;
        bool hash_valid = true;
        
        for (char c : client_salt) {
            if (!isxdigit(c)) {
                salt_valid = false;
                logger.log("err: Salt contains non-hex char: " + std::string(1, c));
                break;
            }
        }
        
        for (char c : client_hash) {
            if (!isxdigit(c)) {
                hash_valid = false;
                logger.log("err: Hash contains non-hex char: " + std::string(1, c));
                break;
            }
        }
        
        if (!salt_valid || !hash_valid) {
            logger.log("err: Invalid salt or hash format");
            send_text("err\n");
            return;
        }
        
        // 6. Проверяем аутентификацию
        if (!verify_authentication(login, client_salt, client_hash)) {
            logger.log("err: Authentication failed");
            send_text("err\n");
            return;
        }
        
        // 7. Отправляем подтверждение
        logger.log("SUCCESS: Authentication OK, sending OK to client");
        send_text("OK\n");
        
        // 8. Получаем количество векторов
        uint32_t vector_count = receive_uint32();
        logger.log("Vector count: " + std::to_string(vector_count));
        
        // 9. Обрабатываем векторы
        for (uint32_t i = 0; i < vector_count; i++) {
            logger.log("--- Processing Vector " + std::to_string(i + 1) + " ---");
            
            // Размер вектора
            uint32_t vector_size = receive_uint32();
            logger.log("Vector size: " + std::to_string(vector_size));
            
            // Данные вектора
            std::vector<int32_t> vector_data = receive_vector(vector_size);
            
            // Логируем значения
            if (!vector_data.empty()) {
                std::string values = "Values: ";
                for (size_t j = 0; j < std::min((size_t)5, vector_data.size()); j++) {
                    values += std::to_string(vector_data[j]) + " ";
                }
                if (vector_data.size() > 5) values += "...";
                logger.log(values);
            }
            
            // Вычисляем произведение
            int32_t product = calculate_vector_product(vector_data);
            logger.log("Product: " + std::to_string(product));
            
            // Отправляем результат
            send_int32(product);
            logger.log("Result sent");
        }
        
        logger.log("=== SESSION COMPLETED ===");
        logger.log("Total vectors processed: " + std::to_string(vector_count));
        
    } catch (const std::exception& e) {
        logger.log("err: " + std::string(e.what()));
        send_text("err\n");
    }
}
