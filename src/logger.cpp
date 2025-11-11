#include "../include/logger.h"
#include <fstream>
#include <ctime>
#include <iomanip>

Logger::Logger(const std::string& filename) : log_file_(filename) {}

void Logger::log(const std::string& message, bool critical) {
    std::ofstream file(log_file_, std::ios_base::app);
    if (file.is_open()) {
        std::string time_str = get_current_time();
        std::string critical_str = critical ? "CRITICAL" : "NON-CRITICAL";
        
        file << "[" << time_str << "] [" << critical_str << "] " << message << std::endl;
        file.close();
    }
}

void Logger::log_error(const std::string& error, bool critical) {
    log("ERROR: " + error, critical);
}

std::string Logger::get_current_time() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buffer);
}
