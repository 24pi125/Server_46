#include <UnitTest++/UnitTest++.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <thread>

SUITE(IntegrationTests) {
    const int TEST_PORT = 33445;
    const std::string TEST_CONFIG = "tests/test_data/integration_clients.conf";
    
    void create_test_environment() {
        system("mkdir -p tests/test_data");
        
        std::ofstream client_file(TEST_CONFIG);
        client_file << "testuser:P@ssl@rd\n";
        client_file.close();
    }
    
    void cleanup_test_environment() {
        system("rm -rf tests/test_data");
        system("pkill -f './server -p 33445' 2>/dev/null");
    }
    
    std::string calculate_md5(const std::string& data) {
        std::string cmd = "echo -n '" + data + "' | md5sum | cut -d' ' -f1";
        char buffer[128];
        std::string result = "";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe) {
            if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                result = buffer;
                result.erase(result.find_last_not_of(" \n\r\t") + 1);
            }
            pclose(pipe);
        }
        return result;
    }
    
    void start_server_in_background() {
        pid_t pid = fork();
        if (pid == 0) {
            // В дочернем процессе запускаем сервер
            execl("./server", "./server", "-p", "33445", 
                  "-d", TEST_CONFIG.c_str(), 
                  "-l", "tests/test_data/integration.log", NULL);
            exit(0);
        }
        sleep(2); // Даем серверу время на запуск
    }
    
    TEST(IT_01_FullAuthenticationCycle) {
        std::cout << "IT-01: Полный цикл аутентификации\n";
        
        create_test_environment();
        start_server_in_background();
        
        // Подключаемся к серверу
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(TEST_PORT);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
        
        int result = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
        CHECK_EQUAL(0, result);
        
        if (result == 0) {
            // Подготовка данных аутентификации
            std::string login = "testuser";
            std::string salt = "1234567890ABCDEF"; // 16 hex символов
            std::string password = "P@ssl@rd";
            std::string hash_input = salt + password;
            std::string hash = calculate_md5(hash_input);
            
            // Отправка данных аутентификации
            std::string auth_data = login + salt + hash;
            send(sock, auth_data.c_str(), auth_data.length(), 0);
            
            // Получение ответа
            char buffer[256];
            int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                std::string response(buffer);
                CHECK(response.find("OK") != std::string::npos);
            }
            
            close(sock);
        }
        
        cleanup_test_environment();
    }
    
    TEST(IT_02_InvalidAuthentication) {
        std::cout << "IT-02: Неудачная аутентификация\n";
        
        create_test_environment();
        start_server_in_background();
        
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(TEST_PORT);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
        
        int result = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
        CHECK_EQUAL(0, result);
        
        if (result == 0) {
            // Отправка неверных данных
            std::string wrong_auth = "wronguser" + std::string(16, '0') + std::string(32, '0');
            send(sock, wrong_auth.c_str(), wrong_auth.length(), 0);
            
            char buffer[256];
            int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                std::string response(buffer);
                CHECK(response.find("err") != std::string::npos);
            }
            
            close(sock);
        }
        
        cleanup_test_environment();
    }
};

int main() {
    std::cout << "========================================\n";
    std::cout << "Integration Tests for Vcalc Server\n";
    std::cout << "========================================\n\n";
    
    system("mkdir -p tests/test_data");
    
    return UnitTest::RunAllTests();
}
