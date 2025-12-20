/**
 * @file session.h
 * @brief Класс для обработки клиентских сессий
 * 
 * Определяет класс Session, который обрабатывает отдельные клиентские подключения.
 * Реализует полный цикл обработки: аутентификация, прием данных, вычисления, отправка результатов.
 * 
 * @note Каждое подключение обрабатывается в отдельном потоке/сессии
 * @see session.cpp
 */

#ifndef SESSION_H
#define SESSION_H

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

class Logger;

/**
 * @brief Класс обработки клиентской сессии
 * 
 * Обрабатывает полный цикл взаимодействия с клиентом:
 * 1. Прием и проверка аутентификационных данных
 * 2. Прием векторов для обработки
 * 3. Вычисление произведений элементов векторов
 * 4. Отправка результатов обратно клиенту
 * 
 * @warning Класс не является потокобезопасным, должен использоваться по одной сессии на поток
 */
class Session {
private:
    int client_socket;                                     ///< Сокет клиента
    std::unordered_map<std::string, std::string>& clients; ///< Ссылка на базу клиентов
    Logger& logger;                                        ///< Ссылка на логгер
    
    // Буфер для приема данных
    std::string receive_buffer;                            ///< Буфер накопленных данных
    
    // Приватные методы
    void receive_to_buffer();                              ///< Принимает данные в буфер
    std::string extract_from_buffer_until_non_hex();       ///< Извлекает hex символы до не-hex
    std::string extract_from_buffer_exact(size_t length);  ///< Извлекает точное количество байт
    
    /**
     * @brief Проверяет аутентификацию клиента
     * 
     * @param login Логин клиента
     * @param salt Соль для хэширования
     * @param received_hash Полученный хэш от клиента
     * @return bool true если аутентификация успешна
     */
    bool verify_authentication(const std::string& login, 
                              const std::string& salt, 
                              const std::string& received_hash);
    
    void process_vectors();                                 ///< Основная логика обработки векторов
    uint32_t receive_uint32();                              ///< Принимает 32-битное беззнаковое число
    std::vector<int32_t> receive_vector(uint32_t size);     ///< Принимает вектор заданного размера
    void send_uint32(uint32_t value);                       ///< Отправляет 32-битное беззнаковое число
    void send_int32(int32_t value);                         ///< Отправляет 32-битное знаковое число
    std::string calculate_md5(const std::string& data);     ///< Вычисляет MD5 хэш
    int32_t calculate_vector_product(const std::vector<int32_t>& vector); ///< Вычисляет произведение вектора

public:
    /**
     * @brief Конструктор сессии
     * 
     * @param client_socket Сокет подключенного клиента
     * @param clients База данных клиентов
     * @param logger Логгер для записи событий
     */
    Session(int client_socket, std::unordered_map<std::string, std::string>& clients, Logger& logger);
    
    /**
     * @brief Основной метод обработки сессии
     * 
     * Запускает обработку клиентской сессии. Выполняет аутентификацию,
     * прием данных, вычисления и отправку результатов.
     * 
     * @note Автоматически закрывает сокет при завершении
     * @throw std::exception при критических ошибках обработки
     */
    void handle();
    
    /**
     * @brief Отправляет текстовые данные клиенту
     * 
     * @param text Текст для отправки
     * @return bool true если отправка успешна, false при ошибке
     */
    bool send_text(const std::string& text);
};

#endif // SESSION_H
