#ifndef LOGGER_H
#define LOGGER_H

#include <string>

class Logger {
private:
    std::string log_file_;
    std::string get_current_time();

public:
    Logger(const std::string& filename);
    void log(const std::string& message, bool critical = false);
    void log_add(const std::string& message);  // <-- ДОБАВИТЬ ЭТУ СТРОКУ
    void log_error(const std::string& error, bool critical = false);
};

#endif // LOGGER_H
