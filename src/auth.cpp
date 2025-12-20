/**
 * @file auth.cpp
 * @brief Реализация методов аутентификации
 * 
 * Содержит реализацию методов класса Authenticator:
 * - генерация случайной соли
 * - вычисление MD5 хэшей
 * - проверка учетных данных клиентов
 * 
 * @see auth.h
 */

#include "../include/auth.h"
#include <openssl/md5.h>
#include <sstream>
#include <iomanip>
#include <random>

/**
 * @brief Генерирует 64-битную соль в виде 16 hex символов
 * 
 * @return std::string Соль в hex формате (16 символов, uppercase)
 * 
 * @details
 * Алгоритм:
 * 1. Создает генератор случайных чисел на основе std::random_device
 * 2. Генерирует 64-битное случайное число
 * 3. Конвертирует в hex строку с заполнением нулями
 * 
 * @note Использует Mersenne Twister для генерации псевдослучайных чисел
 */
std::string Authenticator::generate_salt_16() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    
    uint64_t salt = dis(gen);
    
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << salt;
    return ss.str();
}

/**
 * @brief Вычисляет MD5 хэш от конкатенации соли и пароля
 * 
 * @param salt Соль (должна быть 16 hex символов)
 * @param password Пароль в открытом виде
 * @return std::string 32-символьный MD5 хэш в hex формате (uppercase)
 * 
 * @details
 * Формула: hash = MD5(salt + password)
 * Использует OpenSSL библиотеку для вычисления MD5
 * 
 * @warning Не используйте в продакшене - MD5 уязвим к коллизиям
 */
std::string Authenticator::calculate_md5_hash(const std::string& salt, 
                                            const std::string& password) {
    std::string data = salt + password;
    
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), hash);
    
    std::stringstream ss;
    ss << std::hex << std::uppercase;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        ss << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

/**
 * @brief Проверяет аутентификационные данные клиента
 * 
 * @param login Логин клиента
 * @param received_hash Хэш, полученный от клиента (32 hex символа)
 * @param salt Соль, использованная при аутентификации (16 hex символов)
 * @param clients База данных клиентов в формате логин:пароль
 * @return bool true если хэши совпадают, false если нет или ошибка
 * 
 * @details
 * Шаги проверки:
 * 1. Поиск логина в базе данных
 * 2. Вычисление ожидаемого хэша: MD5(salt + stored_password)
 * 3. Сравнение полученного и вычисленного хэшей
 * 
 * @note Возвращает false если логин не найден или хэши не совпадают
 */
bool Authenticator::verify_client(const std::string& login, 
                                const std::string& received_hash, 
                                const std::string& salt, 
                                const std::unordered_map<std::string, std::string>& clients) {
    auto it = clients.find(login);
    if (it == clients.end()) {
        return false;
    }
    
    std::string calculated_hash = calculate_md5_hash(salt, it->second);
    return calculated_hash == received_hash;
}
