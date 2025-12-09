#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <openssl/md5.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>

class VectorTestClient {
private:
    int sock;
    struct sockaddr_in serv_addr;
    
    std::string calculate_md5(const std::string& data) {
        unsigned char hash[MD5_DIGEST_LENGTH];
        MD5((unsigned char*)data.c_str(), data.length(), hash);
        
        std::stringstream ss;
        ss << std::hex << std::uppercase;
        for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            ss << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    bool send_all(const std::string& data) {
        size_t total_sent = 0;
        const char* buffer = data.c_str();
        size_t length = data.length();
        
        while (total_sent < length) {
            ssize_t sent = send(sock, buffer + total_sent, length - total_sent, 0);
            if (sent <= 0) {
                return false;
            }
            total_sent += sent;
        }
        
        return true;
    }
    
    std::string receive_line() {
        std::string result;
        char ch;
        
        while (true) {
            ssize_t bytes_received = recv(sock, &ch, 1, 0);
            if (bytes_received <= 0) {
                return "";
            }
            
            if (ch == '\n') {
                break;
            }
            
            result += ch;
        }
        
        return result;
    }

public:
    VectorTestClient(const std::string& ip = "127.0.0.1", int port = 33333) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            throw std::runtime_error("Socket creation failed");
        }
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        
        if(inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
            throw std::runtime_error("Invalid address");
        }
    }
    
    bool connect() {
        return ::connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) >= 0;
    }
    
    void close() {
        ::close(sock);
    }
    
    bool authenticate(const std::string& login, const std::string& password) {
        std::cout << "=== AUTHENTICATION PROTOCOL ===" << std::endl;
        
        // Отправляем всё за один раз: login + salt + hash
        std::string salt = "1234567890ABCDEF";  // 16 hex символов
        std::cout << "1. Using salt: " << salt << std::endl;
        std::cout << "2. Login: '" << login << "'" << std::endl;
        
        // Вычисляем хэш: MD5(salt + password)
        std::string hash_input = salt + password;
        std::string hash = calculate_md5(hash_input);
        std::cout << "3. MD5(salt + password): " << hash << std::endl;
        
        // Формируем сообщение аутентификации
        std::string auth_data = login + salt + hash;
        std::cout << "4. Sending: login + salt + hash = '" 
                  << login << "' + '" << salt << "' + '" << hash << "'" << std::endl;
        
        // Отправляем всё за один раз
        if (!send_all(auth_data)) {
            std::cout << "ERROR: Failed to send auth data" << std::endl;
            return false;
        }
        
        // Получаем ответ (OK или err)
        std::string response = receive_line();
        std::cout << "5. Server response: '" << response << "'" << std::endl;
        
        if (response.find("OK") != std::string::npos) {
            std::cout << "✓ AUTHENTICATION SUCCESSFUL!" << std::endl;
            return true;
        } else {
            std::cout << "✗ AUTHENTICATION FAILED!" << std::endl;
            return false;
        }
    }
    
    // Публичные методы для работы с данными
    void send_uint32(uint32_t value) {
        value = htonl(value);
        send(sock, &value, sizeof(value), 0);
    }
    
    void send_int32(int32_t value) {
        value = htonl(value);
        send(sock, &value, sizeof(value), 0);
    }
    
    int32_t receive_int32() {
        int32_t value;
        recv(sock, &value, sizeof(value), 0);
        return ntohl(value);
    }
};

void print_help() {
    std::cout << "Vector Processing Test Client" << std::endl;
    std::cout << "Usage: test_client [options]" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  -h              Show this help message" << std::endl;
    std::cout << "  -a <ip>         Server IP address (default: 127.0.0.1)" << std::endl;
    std::cout << "  -p <port>       Server port (default: 33333)" << std::endl;
    std::cout << "  -u <user>       Username (default: user)" << std::endl;
    std::cout << "  -w <password>   Password (default: P@ssw0rd)" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  test_client -u user -w P@ssw0rd" << std::endl;
    std::cout << "  test_client -a 192.168.1.100 -p 33333 -u user -w P@ssw0rd" << std::endl;
    std::cout << "  test_client 127.0.0.1 33444  # Legacy format support" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string ip = "127.0.0.1";
    int port = 33333;
    std::string user = "user";
    std::string password = "P@ssW0rd";
    
    // Поддержка старого формата: ./test_client <host> <port>
    if (argc == 3) {
        ip = argv[1];
        try {
            port = std::stoi(argv[2]);
        } catch (...) {
            std::cerr << "ERROR: Invalid port number" << std::endl;
            return 1;
        }
        std::cout << "Using legacy format: host=" << ip << ", port=" << port << std::endl;
    }
    // Поддержка нового формата с опциями
    else if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            
            if (arg == "-h") {
                print_help();
                return 0;
            } else if (arg == "-a" && i + 1 < argc) {
                ip = argv[++i];
            } else if (arg == "-p" && i + 1 < argc) {
                try {
                    port = std::stoi(argv[++i]);
                } catch (...) {
                    std::cerr << "ERROR: Invalid port number" << std::endl;
                    return 1;
                }
            } else if (arg == "-u" && i + 1 < argc) {
                user = argv[++i];
            } else if (arg == "-w" && i + 1 < argc) {
                password = argv[++i];
            } else {
                std::cerr << "ERROR: Unknown option: " << arg << std::endl;
                print_help();
                return 1;
            }
        }
    }
    
    try {
        std::cout << "=== VECTOR PROCESSING CLIENT ===" << std::endl;
        std::cout << "Server: " << ip << ":" << port << std::endl;
        std::cout << "User: " << user << std::endl;
        
        VectorTestClient client(ip, port);
        
        std::cout << "\nConnecting to server..." << std::endl;
        if (!client.connect()) {
            std::cerr << "ERROR: Connection failed" << std::endl;
            return 1;
        }
        
        std::cout << "✓ Connected successfully" << std::endl;
        
        // Тестируем аутентификацию
        if (!client.authenticate(user, password)) {
            std::cerr << "ERROR: Authentication failed" << std::endl;
            return 1;
        }
        
        // Тестовые векторы
        std::vector<std::vector<int32_t>> test_vectors = {
            {1, 2, 3, 4},        // 1*2*3*4 = 24
            {5, -2, 3},          // 5*(-2)*3 = -30
            {10, 20, 0, 30},     // 10*20*0*30 = 0
            {100, 200},          // 100*200 = 20000
        };
        
        std::vector<int32_t> results;
        std::cout << "\n=== VECTOR PROCESSING ===" << std::endl;
        
        // Отправляем количество векторов
        uint32_t vector_count = test_vectors.size();
        std::cout << "1. Sending number of vectors: " << vector_count << std::endl;
        client.send_uint32(vector_count);
        
        std::cout << "2. Processing vectors..." << std::endl;
        
        // Для каждого вектора: отправляем, сразу получаем результат
        for (size_t i = 0; i < test_vectors.size(); i++) {
            const auto& vector = test_vectors[i];
            
            // Отправляем размер вектора
            uint32_t vector_size = vector.size();
            std::cout << "   Vector " << (i+1) << " size: " << vector_size;
            client.send_uint32(vector_size);
            
            // Отправляем значения вектора
            std::cout << ", values: [";
            for (size_t j = 0; j < vector.size(); j++) {
                client.send_int32(vector[j]);
                std::cout << vector[j];
                if (j < vector.size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
            
            // СРАЗУ получаем результат для этого вектора
            int32_t result = client.receive_int32();
            results.push_back(result);
            std::cout << "   Result " << (i+1) << ": " << result << std::endl;
        }
        
        // Проверяем результаты
        std::cout << "\n=== FINAL RESULTS ===" << std::endl;
        bool all_correct = true;
        
        for (size_t i = 0; i < results.size(); i++) {
            // Вычисляем ожидаемый результат
            int64_t expected = 1;
            for (int32_t val : test_vectors[i]) {
                expected *= val;
                
                // Проверка на переполнение (как в сервере)
                if (expected > 2147483647LL) {
                    expected = 2147483647LL;  // INT32_MAX
                    break;
                }
                if (expected < -2147483648LL) {
                    expected = -2147483648LL; // INT32_MIN
                    break;
                }
            }
            
            bool correct = (results[i] == static_cast<int32_t>(expected));
            
            std::cout << "Vector " << (i+1) << ": [";
            for (size_t j = 0; j < test_vectors[i].size(); j++) {
                std::cout << test_vectors[i][j];
                if (j < test_vectors[i].size() - 1) std::cout << ", ";
            }
            std::cout << "] = " << results[i];
            
            if (correct) {
                std::cout << " ✓ CORRECT";
                if (static_cast<int64_t>(results[i]) != expected) {
                    std::cout << " (overflow handled)";
                }
            } else {
                std::cout << " ✗ WRONG (expected: " << static_cast<int32_t>(expected) << ")";
                all_correct = false;
            }
            std::cout << std::endl;
        }
        
        client.close();
        std::cout << "\n✓ Connection closed" << std::endl;
        
        if (all_correct) {
            std::cout << "\n✓ All tests passed!" << std::endl;
            return 0;
        } else {
            std::cout << "\n✗ Some tests failed" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ ERROR: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
