#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>

class ServerTester {
private:
    int server_port;
    std::string config_file;
    std::string log_file;
    pid_t server_pid;
    
public:
    ServerTester(int port = 33444) : server_port(port), server_pid(-1) {
        config_file = "/tmp/test_server_config_" + std::to_string(getpid()) + ".conf";
        log_file = "/tmp/test_server_log_" + std::to_string(getpid()) + ".log";
    }
    
    ~ServerTester() {
        stop_server();
        cleanup();
    }
    
    void create_test_config() {
        std::ofstream config(config_file);
        if (config.is_open()) {
            config << "# Test authentication database\n";
            config << "user:P@ssW0rd\n";
            config << "alice:P@ssl@rd\n";
            config << "bob:secret456\n";
            config.close();
        }
    }
    
    bool start_server() {
        create_test_config();
        
        std::string command = "./server -p " + std::to_string(server_port) + 
                             " -c " + config_file + 
                             " -l " + log_file + 
                             " > /tmp/server_output.log 2>&1 &";
        
        std::cout << "Starting server on port " << server_port << std::endl;
        
        int result = system(command.c_str());
        if (result == -1) {
            std::cerr << "Failed to start server" << std::endl;
            return false;
        }
        
        // Даем серверу время запуститься
        sleep(2);
        
        // Пытаемся найти PID сервера
        std::string pid_command = "ps aux | grep './server -p " + 
                                 std::to_string(server_port) + 
                                 "' | grep -v grep | awk '{print $2}'";
        
        FILE* pipe = popen(pid_command.c_str(), "r");
        if (pipe) {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                server_pid = std::stoi(buffer);
                std::cout << "Server PID: " << server_pid << std::endl;
            }
            pclose(pipe);
        }
        
        return (server_pid > 0);
    }
    
    bool stop_server() {
        if (server_pid > 0) {
            std::cout << "Stopping server with PID: " << server_pid << std::endl;
            
            // Посылаем SIGTERM
            kill(server_pid, SIGTERM);
            sleep(1);
            
            // Проверяем, остановился ли
            if (kill(server_pid, 0) == 0) {
                // Если еще работает, посылаем SIGKILL
                kill(server_pid, SIGKILL);
                sleep(1);
            }
            
            server_pid = -1;
        }
        return true;
    }
    
    bool is_server_running() {
        if (server_pid <= 0) return false;
        
        // Проверяем существует ли процесс
        if (kill(server_pid, 0) != 0) {
            server_pid = -1;
            return false;
        }
        
        // Пробуем подключиться к порту для проверки
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            return false;
        }
        
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(server_port);
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
        
        // Устанавливаем таймаут
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        
        bool is_running = (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0);
        close(sock);
        
        return is_running;
    }
    
    void check_server_log() {
        std::cout << "\n=== Server Log (last 5 lines) ===" << std::endl;
        std::ifstream logfile(log_file);
        if (logfile.is_open()) {
            std::string line;
            std::vector<std::string> lines;
            
            while (std::getline(logfile, line)) {
                lines.push_back(line);
            }
            
            int start = std::max(0, (int)lines.size() - 5);
            for (int i = start; i < (int)lines.size(); i++) {
                std::cout << lines[i] << std::endl;
            }
            
            if (lines.empty()) {
                std::cout << "Log file is empty" << std::endl;
            }
            
            logfile.close();
        } else {
            std::cout << "Log file not found or cannot be opened" << std::endl;
        }
    }
    
    void cleanup() {
        // Удаляем временные файлы
        std::remove(config_file.c_str());
        std::remove(log_file.c_str());
        std::remove("/tmp/server_output.log");
        
        // Очищаем другие временные файлы
        std::string cleanup_cmd = "rm -f /tmp/test_* /tmp/server_* 2>/dev/null";
        system(cleanup_cmd.c_str());
    }
    
    bool run_integration_test() {
        std::cout << "\n=================================" << std::endl;
        std::cout << "Integration Test - Server" << std::endl;
        std::cout << "=================================" << std::endl;
        
        std::cout << "\n1. Starting server..." << std::endl;
        if (!start_server()) {
            std::cerr << "✗ Failed to start server" << std::endl;
            return false;
        }
        std::cout << "✓ Server started" << std::endl;
        
        std::cout << "\n2. Checking server status..." << std::endl;
        if (!is_server_running()) {
            std::cerr << "✗ Server is not running or not accessible" << std::endl;
            stop_server();
            return false;
        }
        std::cout << "✓ Server is running and accessible on port " << server_port << std::endl;
        
        check_server_log();
        
        std::cout << "\n3. Running client tests..." << std::endl;
        std::string test_command = "./test_client -a 127.0.0.1 -p " + std::to_string(server_port) + " -u user -w P@ssW0rd";
        std::cout << "Executing: " << test_command << std::endl;
        
        int client_result = system(test_command.c_str());
        
        if (client_result != 0) {
            std::cerr << "✗ Client tests failed with code: " << client_result << std::endl;
            stop_server();
            return false;
        }
        std::cout << "✓ Client tests passed" << std::endl;
        
        std::cout << "\n4. Stopping server..." << std::endl;
        if (!stop_server()) {
            std::cerr << "✗ Failed to stop server" << std::endl;
            return false;
        }
        std::cout << "✓ Server stopped" << std::endl;
        
        check_server_log();
        
        return true;
    }
};

int main() {
    std::cout << "Server Integration Test" << std::endl;
    std::cout << "========================" << std::endl;
    
    ServerTester tester(33444); // Используем порт 33444 для тестов
    
    if (tester.run_integration_test()) {
        std::cout << "\n=================================" << std::endl;
        std::cout << "✓ Integration test PASSED" << std::endl;
        std::cout << "=================================" << std::endl;
        return 0;
    } else {
        std::cout << "\n=================================" << std::endl;
        std::cout << "✗ Integration test FAILED" << std::endl;
        std::cout << "=================================" << std::endl;
        return 1;
    }
}
