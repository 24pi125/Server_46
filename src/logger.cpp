/**
 * @file logger.cpp
 * @brief Реализация класса Logger
 * 
 * Содержит реализацию методов логирования:
 * - создание логгера с указанием файла
 * - запись сообщений с временными метками
 * - обработка ошибок и критических событий
 * - добавление текста к существующей записи
 * 
 * @see logger.h
 */

#include "../include/logger.h"
#include <fstream>
#include <ctime>
#include <iomanip>
#include <iostream>

/**
 * @brief Конструктор логгера
 * 
 * @param filename Путь к файлу для записи логов
 * 
 * @note Не создает файл, если он не существует - файл создается при первой записи
 */
Logger::Logger(const std::string& filename) : log_file_(filename) {}

/**
 * @brief Записывает сообщение в лог
 * 
 * @param message Текст сообщения
 * @param critical Флаг критичности
 * 
 * @details
 * 1. Выводит сообщение в стандартный вывод (std::cout)
 * 2. Открывает файл логов в режиме добавления
 * 3. Форматирует запись: [YYYY-MM-DD HH:MM:SS] [LEVEL] message
 * 4. Записывает в файл и закрывает его
 * 
 * @note Потокобезопасность не гарантируется
 */
void Logger::log(const std::string& message, bool critical) {
    // Вывод в консоль для отладки
    std::cout << message << std::endl;
    
    // Запись в файл
    std::ofstream file(log_file_, std::ios_base::app);
    if (file.is_open()) {
        std::string time_str = get_current_time();
        std::string critical_str = critical ? "CRITICAL" : "NON-CRITICAL";

        file << "[" << time_str << "] [" << critical_str << "] " << message << std::endl;
        file.close();
    }
}

/**
 * @brief Добавляет текст к последней записи лога
 * 
 * @param message Текст для добавления
 * 
 * @details
 * Отличается от log() тем, что:
 * - Не добавляет временную метку
 * - Не добавляет новую строку в конце
 * - Выводит в консоль без std::endl
 * 
 * @note Полезно для создания прогресс-баров или форматированных выводов
 */
void Logger::log_add(const std::string& message) {
    // Вывод в консоль без новой строки
    std::cout << message;
    
    // Запись в файл без новой строки
    std::ofstream file(log_file_, std::ios_base::app);
    if (file.is_open()) {
        file << message;
        file.close();
    }
}

/**
 * @brief Записывает сообщение об ошибке
 * 
 * @param error Текст ошибки
 * @param critical Флаг критичности
 * 
 * @details
 * Добавляет префикс "err: " к сообщению об ошибке
 * и вызывает log() для фактической записи
 * 
 * @example
 * logger.log_error("Connection failed", true);
 * // Запишет: [2024-01-15 10:30:00] [CRITICAL] err: Connection failed
 */
void Logger::log_error(const std::string& error, bool critical) {
    // err вместо ERROR
    std::string err_message = "err: " + error;
    log(err_message, critical);
}

/**
 * @brief Получает текущее время в формате строки
 * 
 * @return std::string Время в формате "YYYY-MM-DD HH:MM:SS"
 * 
 * @note Использует локальное время системы
 * @note Формат соответствует ISO 8601 без временной зоны
 */
std::string Logger::get_current_time() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buffer);
}
