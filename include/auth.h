/**
 * @file auth.h
 * @brief Класс для аутентификации клиентов
 * 
 * Содержит класс Authenticator, который предоставляет статические методы
 * для генерации соли, расчета MD5 хэшей и проверки подлинности клиентов.
 * 
 * @note Использует алгоритм MD5 для хэширования паролей с солью
 * @warning MD5 считается криптографически слабым, использовать только в тестовых целях
 */

#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <unordered_map>

/**
 * @brief Класс аутентификации клиентов
 * 
 * Предоставляет статические методы для работы с аутентификацией:
 * - генерация 64-битной соли в виде 16 hex символов
 * - вычисление MD5 хэша по схеме salt+password
 * - верификация клиентских учетных данных
 */
class Authenticator {
public:
    /**
     * @brief Генерирует 64-битную соль в виде 16 hex символов
     * 
     * @return std::string 16-символьная hex строка соли
     * 
     * @note Использует случайные числа через std::random_device
     * @warning Соль должна быть уникальной для каждой сессии
     */
    static std::string generate_salt_16();
    
    /**
     * @brief Вычисляет MD5 хэш от конкатенации соли и пароля
     * 
     * @param salt Соль (16 hex символов)
     * @param password Пароль в открытом виде
     * @return std::string 32-символьный MD5 хэш в hex формате
     * 
     * @note Порядок: salt + password (соль добавляется перед паролем)
     * @warning Для реальных проектов используйте более стойкие алгоритмы (SHA-256, bcrypt)
     */
    static std::string calculate_md5_hash(const std::string& salt, 
                                          const std::string& password);
    
    /**
     * @brief Проверяет аутентификационные данные клиента
     * 
     * @param login Логин клиента
     * @param received_hash Хэш, полученный от клиента
     * @param salt Соль, использованная при аутентификации
     * @param clients База данных клиентов (логин -> пароль)
     * @return bool true если аутентификация успешна, иначе false
     * 
     * @note Сравнивает полученный хэш с вычисленным MD5(salt + stored_password)
     * @throw Ничего не выбрасывает, возвращает false при ошибках
     */
    static bool verify_client(const std::string& login, 
                              const std::string& received_hash, 
                              const std::string& salt, 
                              const std::unordered_map<std::string, std::string>& clients);
};

#endif
