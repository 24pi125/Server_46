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
#include <random>  // Добавляем

class TestServerClient {
private:
    int sock;
    struct sockaddr_in serv_addr;
    
    std::string calculate_md5(const std::string& data) {
        unsigned char hash[MD5_DIGEST_LENGTH];
        MD5((unsigned char*)data.c_str(), data.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    bool send_string(const std::string& str) {
        ssize_t bytes_sent = send(sock, str.c_str(), str.length(), 0);
        return bytes_sent > 0;
    }
    
    bool receive_string(std::string& str, size_t max_len = 256) {
        char buffer[max_len];
        memset(buffer, 0, max_len);
        ssize_t bytes_received = recv(sock, buffer, max_len - 1, 0);
        if (bytes_received <= 0) {
            return false;
        }
        str = std::string(buffer);
        str.erase(str.find_last_not_of(" \n\r\t") + 1);
        return true;
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
    TestServerClient(const std::string& ip = "127.0.0.1", int port = 33333) {
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
    
    bool authenticate(bool salt_from_client = false) {
        std::cout << "=== AUTHENTICATION ===" << std::endl;
        
        // 1. Отправляем логин
        std::cout << "1. Sending login: user" << std::endl;
        if (!send_string("user")) {
            std::cout << "ERROR: Failed to send login" << std::endl;
            return false;
        }
        
        std::string response;
        
        if (salt_from_client) {
            // Если соль от клиента
            // Получаем подтверждение логина
            if (!receive_string(response)) {
                std::cout << "ERROR: No response to login" << std::endl;
                return false;
            }
            
            if (response == "ERR") {
                std::cout << "ERROR: Invalid login" << std::endl;
                return false;
            }
            
            // Генерируем соль
            std::random_device rd;
            std::mt19937_64 gen(rd());
            std::uniform_int_distribution<uint64_t> dis;
            uint64_t salt_val = dis(gen);
            std::stringstream ss;
            ss << std::hex << std::setw(16) << std::setfill('0') << salt_val;
            std::string salt = ss.str();
            
            std::cout << "2. Generated and sending salt: " << salt << std::endl;
            if (!send_string(salt)) {
                std::cout << "ERROR: Failed to send salt" << std::endl;
                return false;
            }
            
            // Отправляем хэш
            std::string hash_input = salt + "P@ssW0rd";
            std::string hash = calculate_md5(hash_input);
            std::cout << "3. Sending hash: " << hash << std::endl;
            if (!send_string(hash)) {
                std::cout << "ERROR: Failed to send hash" << std::endl;
                return false;
            }
            
        } else {
            // Стандартный режим: соль от сервера
            if (!receive_string(response)) {
                std::cout << "ERROR: No salt from server" << std::endl;
                return false;
            }
            
            if (response == "ERR") {
                std::cout << "ERROR: Invalid login" << std::endl;
                return false;
            }
            
            std::string salt = response;
            std::cout << "2. Received salt from server: " << salt << std::endl;
            
            std::string hash_input = salt + "P@ssW0rd";
            std::string hash = calculate_md5(hash_input);
            std::cout << "3. Sending hash: " << hash << std::endl;
            if (!send_string(hash)) {
                std::cout << "ERROR: Failed to send hash" << std::endl;
                return false;
            }
        }
        
        if (!receive_string(response)) {
            std::cout << "ERROR: No authentication response" << std::endl;
            return false;
        }
        
        std::cout << "4. Authentication result: " << response << std::endl;
        
        if (response == "OK") {
            std::cout << "✓ AUTHENTICATION SUCCESSFUL!" << std::endl;
            return true;
        } else {
            std::cout << "✗ AUTHENTICATION FAILED!" << std::endl;
            return false;
        }
    }
    
    void send_test_vectors() {
        std::vector<std::vector<int32_t>> test_vectors = {
            {1, 2, 3, 4},
            {5, 6, 7, 8},
            {10, 20, 30, 40},
            {100, 200, 300, 400}
        };
        
        std::cout << "\nSending test vectors:" << std::endl;
        for (size_t i = 0; i < test_vectors.size(); i++) {
            std::cout << "Vector " << (i + 1) << ": [";
            for (size_t j = 0; j < test_vectors[i].size(); j++) {
                std::cout << test_vectors[i][j];
                if (j < test_vectors[i].size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
        }
        
        send_uint32(test_vectors.size());
        
        for (const auto& vector : test_vectors) {
            send_uint32(vector.size());
            
            for (int32_t value : vector) {
                send_int32(value);
            }
        }
    }
    
    std::vector<int32_t> receive_results() {
        std::vector<int32_t> results;
        
        uint32_t result_count = receive_uint32();
        results.reserve(result_count);
        
        for (uint32_t i = 0; i < result_count; i++) {
            results.push_back(receive_int32());
        }
        
        return results;
    }
};

int main(int argc, char* argv[]) {
    bool salt_from_client = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-S" && i + 1 < argc) {
            std::string side = argv[++i];
            if (side == "client" || side == "c") {
                salt_from_client = true;
            }
        }
    }
    
    try {
        TestServerClient client("127.0.0.1", 33333);
        
        std::cout << "Connecting to test server at 127.0.0.1:33333..." << std::endl;
        if (!client.connect()) {
            std::cerr << "Connection failed" << std::endl;
            return 1;
        }
        
        std::cout << "Connected to server" << std::endl;
        
        if (!client.authenticate(salt_from_client)) {
            return 1;
        }
        
        client.send_test_vectors();
        
        std::cout << "\nReceiving results..." << std::endl;
        std::vector<int32_t> results = client.receive_results();
        
        std::cout << "\n=== RESULTS ===" << std::endl;
        for (size_t i = 0; i < results.size(); i++) {
            std::cout << "Vector " << (i + 1) << " product: " << results[i] << std::endl;
        }
        
        client.close();
        std::cout << "\nTest completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
