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
    
    bool send_text(const std::string& text) {
        ssize_t bytes_sent = send(sock, text.c_str(), text.length(), 0);
        return bytes_sent == (ssize_t)text.length();
    }
    
    bool send_text_with_newline(const std::string& text) {
        std::string data = text + "\n";
        return send_text(data);
    }
    
    std::string receive_text_exact(size_t length) {
        char buffer[length + 1];
        memset(buffer, 0, length + 1);
        ssize_t bytes_received = recv(sock, buffer, length, 0);
        if (bytes_received != (ssize_t)length) {
            return "";
        }
        return std::string(buffer, length);
    }
    
    std::string receive_text_until_newline() {
        std::string result;
        char ch;
        
        while (true) {
            ssize_t bytes_received = recv(sock, &ch, 1, 0);
            if (bytes_received <= 0) {
                return "";
            }
            
            if (ch == '\n' || ch == '\r' || ch == '\0') {
                break;
            }
            
            result += ch;
            
            // Защита от слишком длинных строк
            if (result.length() > 255) {
                return "";
            }
        }
        
        // Убираем пробелы в конце
        result.erase(result.find_last_not_of(" \t") + 1);
        return result;
    }
    
    void send_uint32(uint32_t value) {
        value = htonl(value);
        send(sock, &value, sizeof(value), 0);
    }
    
    void send_int32(int32_t value) {
        value = htonl(value);
        send(sock, &value, sizeof(value), 0);
    }
    
    uint32_t receive_uint32() {
        uint32_t value;
        recv(sock, &value, sizeof(value), 0);
        return ntohl(value);
    }
    
    int32_t receive_int32() {
        int32_t value;
        recv(sock, &value, sizeof(value), 0);
        return ntohl(value);
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
        
        // Шаг 2: Отправляем логин с \n
        std::cout << "1. Sending login: '" << login << "'" << std::endl;
        if (!send_text_with_newline(login)) {
            std::cout << "ERROR: Failed to send login" << std::endl;
            return false;
        }
        
        // Шаг 3: Получаем ответ на логин
        std::string response = receive_text_until_newline();
        
        if (response == "ERR") {
            // 3б: Ошибка идентификации
            std::cout << "2. Server response: ERR (invalid login)" << std::endl;
            std::cout << "✗ CONNECTION TERMINATED" << std::endl;
            return false;
        }
        
        // 3а: Должны получить соль (16 hex символов)
        if (response.length() != 16) {
            std::cout << "ERROR: Invalid salt received: '" << response << "'" << std::endl;
            std::cout << "  Expected 16 hex characters, got " << response.length() << std::endl;
            return false;
        }
        
        std::string salt = response;
        std::cout << "2. Received salt (16 hex): " << salt << std::endl;
        
        // Проверяем что соль состоит только из hex символов
        for (char c : salt) {
            if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
                std::cout << "ERROR: Salt contains non-hex character: " << c << std::endl;
                return false;
            }
        }
        
        // Шаг 4: Вычисляем и отправляем хэш
        std::string hash_input = salt + password;
        std::string hash = calculate_md5(hash_input);
        std::cout << "3. Sending MD5 hash: " << hash << std::endl;
        if (!send_text(hash)) {
            std::cout << "ERROR: Failed to send hash" << std::endl;
            return false;
        }
        
        // Шаг 5: Получаем результат аутентификации
        response = receive_text_until_newline();
        std::cout << "4. Authentication result: " << response << std::endl;
        
        if (response == "OK") {
            std::cout << "✓ AUTHENTICATION SUCCESSFUL!" << std::endl;
            return true;
        } else {
            std::cout << "✗ AUTHENTICATION FAILED!" << std::endl;
            std::cout << "  Connection terminated by server" << std::endl;
            return false;
        }
    }
    
    void send_vectors(const std::vector<std::vector<int32_t>>& vectors) {
        std::cout << "\n=== VECTOR PROCESSING ===" << std::endl;
        
        // Шаг 6: Отправляем количество векторов
        uint32_t vector_count = vectors.size();
        std::cout << "5. Sending number of vectors: " << vector_count << std::endl;
        send_uint32(vector_count);
        
        for (size_t i = 0; i < vectors.size(); i++) {
            const auto& vector = vectors[i];
            
            // Шаг 7: Отправляем размер вектора
            uint32_t vector_size = vector.size();
            std::cout << "6." << (i+1) << " Vector " << (i+1) << " size: " << vector_size << std::endl;
            send_uint32(vector_size);
            
            // Шаг 8: Отправляем значения вектора
            std::cout << "   Values: [";
            for (size_t j = 0; j < vector.size(); j++) {
                send_int32(vector[j]);
                std::cout << vector[j];
                if (j < vector.size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
        }
    }
    
    std::vector<int32_t> receive_results() {
        std::vector<int32_t> results;
        
        // Получаем количество результатов
        uint32_t result_count = receive_uint32();
        std::cout << "\n7. Receiving " << result_count << " results..." << std::endl;
        results.reserve(result_count);
        
        // Получаем каждый результат
        for (uint32_t i = 0; i < result_count; i++) {
            int32_t result = receive_int32();
            results.push_back(result);
            std::cout << "   Result " << (i+1) << ": " << result << std::endl;
        }
        
        return results;
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
}

int main(int argc, char* argv[]) {
    std::string ip = "127.0.0.1";
    int port = 33333;
    std::string user = "user";
    std::string password = "P@ssW0rd";
    
    // Парсим аргументы командной строки
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
        
        if (!client.authenticate(user, password)) {
            return 1;
        }
        
        // Тестовые векторы
        std::vector<std::vector<int32_t>> test_vectors = {
            {1, 2, 3, 4},                    // 1*2*3*4 = 24
            {5, 6},                          // 5*6 = 30
            {10, 20, 30},                    // 10*20*30 = 6000
            {2, 3, 5, 7, 11},                // 2*3*5*7*11 = 2310
            {1000, 1000, 1000},              // 1000^3 = 1,000,000,000
            {-1, 2, -3, 4},                  // -1*2*-3*4 = 24
            {0, 5, 10},                      // 0*5*10 = 0
            {2147483647, 2},                 // Переполнение → 2^32-1
            {1000000, 1000000, 1000000}      // 1,000,000^3 = 1,000,000,000,000,000
        };
        
        client.send_vectors(test_vectors);
        
        std::vector<int32_t> results = client.receive_results();
        
        std::cout << "\n=== FINAL RESULTS ===" << std::endl;
        for (size_t i = 0; i < results.size(); i++) {
            // Вычисляем ожидаемый результат для проверки
            int64_t expected = 1;
            bool overflow = false;
            
            for (int32_t val : test_vectors[i]) {
                expected *= val;
                if (expected > 4294967295LL) {
                    expected = 4294967295LL; // 2^32-1
                    overflow = true;
                }
                if (expected < -4294967296LL) {
                    expected = -4294967296LL; // -2^32
                    overflow = true;
                }
            }
            
            std::cout << "Vector " << (i+1) << ": [";
            for (size_t j = 0; j < test_vectors[i].size(); j++) {
                std::cout << test_vectors[i][j];
                if (j < test_vectors[i].size() - 1) std::cout << ", ";
            }
            std::cout << "] = " << results[i];
            
            if (results[i] == static_cast<int32_t>(expected)) {
                std::cout << " ✓ CORRECT";
                if (overflow) std::cout << " (overflow handled)";
            } else {
                std::cout << " ✗ WRONG (expected: " << static_cast<int32_t>(expected) << ")";
            }
            std::cout << std::endl;
        }
        
        client.close();
        std::cout << "\n✓ Test completed successfully!" << std::endl;
        std::cout << "✓ Connection closed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ ERROR: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
