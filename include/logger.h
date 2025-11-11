#ifndef LOGGER_H
#define LOGGER_H

#include <string>

class Logger {
private:
    std::string log_file_;
    
public:
    Logger(const std::string& filename);
    void log(const std::string& message, bool critical = false);
    void log_error(const std::string& error, bool critical = false);
    
private:
    std::string get_current_time();
};

#endif
