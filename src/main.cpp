/**
 * @file main.cpp
 * @brief Точка входа в программу сервера
 * 
 * Содержит функцию main() - точку входа в приложение сервера.
 * Отвечает за парсинг аргументов командной строки, создание
 * и запуск экземпляра сервера с указанной конфигурацией.
 * 
 * @note Всегда выводит справку по использованию при запуске
 */

#include <iostream>
#include <string>
#include "server.h"
#include "config.h"

/**
 * @brief Точка входа в программу сервера
 * 
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 * @return int Код возврата: 0 - успех, 1 - ошибка
 * 
 * @details
 * Последовательность выполнения:
 * 1. Парсинг аргументов командной строки в конфигурацию
 * 2. Вывод справки по использованию (всегда)
 * 3. Вывод текущей конфигурации
 * 4. Создание и запуск экземпляра сервера
 * 5. Обработка исключений и возврат соответствующего кода
 * 
 * @example
 * ./server                    # Запуск с настройками по умолчанию
 * ./server -p 44444           # Запуск на порту 44444
 * ./server -c ./clients.conf  # Использование кастомного файла клиентов
 */
int main(int argc, char* argv[]) {
    try {
        // Парсим аргументы командной строки
        ServerConfig config = ServerConfig::parse_args(argc, argv);
        
        // ВСЕГДА показываем справку при запуске
        std::cout << "Usage: ./server [options]\n";
        std::cout << "\n";
        std::cout << "Options:\n";
        std::cout << "  -h, --help       Show this help message\n";
        std::cout << "  -p PORT          Server port (default: 33333)\n";
        std::cout << "  -c CONFIG_FILE   Client database file (default: /etc/vealc.conf)\n";
        std::cout << "  -d CONFIG_FILE   Alias for -c\n";
        std::cout << "  -l LOG_FILE      Log file (default: /var/log/vealc.log)\n";
        std::cout << "\n";
        std::cout << "Examples:\n";
        std::cout << "  ./server                    # Run with default settings\n";
        std::cout << "  ./server -p 44444           # Run on port 44444\n";
        std::cout << "  ./server -c ./myconfig.conf # Use custom config file\n";
        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "Starting server with configuration:\n";
        std::cout << "  Port: " << config.port << "\n";
        std::cout << "  Config file: " << config.client_db_file << "\n";
        std::cout << "  Log file: " << config.log_file << "\n";
        std::cout << "========================================\n\n";
        
        // Запускаем сервер с конфигом
        Server server(config);
        server.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error\n";
        return 1;
    }
    
    return 0;
}
