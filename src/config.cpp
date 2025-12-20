/**
 * @file config.cpp
 * @brief Реализация методов конфигурации сервера
 * 
 * Содержит реализацию парсинга аргументов командной строки
 * и отображения справки по использованию сервера.
 * 
 * @see config.h
 */

#include "../include/config.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <limits>

/**
 * @brief Парсит аргументы командной строки
 * 
 * @param argc Количество аргументов
 * @param argv Массив аргументов
 * @return ServerConfig Заполненная структура конфигурации
 * 
 * @details
 * Обрабатывает следующие опции:
 * -h, --help       -> выводит справку и завершает программу
 * -p PORT          -> устанавливает порт сервера
 * -c FILE          -> указывает файл конфигурации клиентов
 * -d FILE          -> синоним для -c
 * -l FILE          -> указывает файл логов
 * 
 * @note При неизвестном аргументе выводит справку и завершает программу с кодом 1
 * @note Если аргументов нет, возвращает конфигурацию по умолчанию
 */
ServerConfig ServerConfig::parse_args(int argc, char* argv[]) {
    ServerConfig config;
    
    // Если нет аргументов - возвращаем конфиг по умолчанию
    if (argc == 1) {
        return config;  // Просто возвращаем дефолтный конфиг
    }
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
            exit(0);
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            config.client_db_file = argv[++i];
        } else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
            config.log_file = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            try {
                int port = std::stoi(argv[++i]);
                // Проверка валидности порта
                if (port <= 0 || port > 65535) {
                    std::cerr << "Error: Port must be between 1 and 65535\n";
                    exit(1);
                }
                config.port = port;
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid port number - " << argv[i] << "\n";
                exit(1);
            }
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            config.client_db_file = argv[++i];
        } else {
            // Неизвестный аргумент
            std::cerr << "Unknown option: " << argv[i] << "\n\n";
            print_help();
            exit(1);
        }
    }
    
    return config;
}

/**
 * @brief Выводит справку по использованию программы
 * 
 * Форматированный вывод доступных опций и примеров использования.
 * Вызывается автоматически при указании -h или --help, а также
 * при передаче неизвестного аргумента.
 */
void ServerConfig::print_help() {
    std::cout << "Usage: ./server [options]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help       Show this help message\n";
    std::cout << "  -p PORT          Server port (1-65535, default: 33333)\n";
    std::cout << "  -c CONFIG_FILE   Client database file (default: /etc/vealc.conf)\n";
    std::cout << "  -d CONFIG_FILE   Alias for -c\n";
    std::cout << "  -l LOG_FILE      Log file (default: /var/log/vealc.log)\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  ./server                    # Run with default settings\n";
    std::cout << "  ./server -p 44444           # Run on port 44444\n";
    std::cout << "  ./server -c ./myconfig.conf # Use custom config file\n";
}
