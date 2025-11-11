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
        for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    void send_string(const std::string& str) {
        send(sock, str.c_str(), str.length(), 0);
    }
    
    std::string receive_string(size_t max_len = 256) {
        char buffer[max_len];
        memset(buffer, 0, max_len);
        recv(sock, buffer, max_len - 1, 0);
        return std::string(buffer);
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
        send_string(login);
        
        std::string response = receive_string();
        
        if (response == "ERR") {
            std::cout << "Authentication failed: invalid login" << std::endl;
            return false;
        }
        
        std::string salt = response;
        std::string hash_input = salt + password;
        std::string hash = calculate_md5(hash_input);
        
        send_string(hash);
        
        response = receive_string();
        if (response == "OK") {
            std::cout << "Authentication successful!" << std::endl;
            return true;
        } else {
            std::cout << "Authentication failed: invalid password" << std::endl;
            return false;
        }
    }
    
    void send_vectors(const std::vector<std::vector<int32_t>>& vectors) {
        send_uint32(vectors.size());
        
        for (const auto& vector : vectors) {
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

void print_help() {
    std::cout << "Usage: test_client [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h              Show this help" << std::endl;
    std::cout << "  -a <ip>         Server IP address (default: 127.0.0.1)" << std::endl;
    std::cout << "  -p <port>       Server port (default: 33333)" << std::endl;
    std::cout << "  -u <user>       Username (default: user1)" << std::endl;
    std::cout << "  -w <password>   Password (default: password1)" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string ip = "127.0.0.1";
    int port = 33333;
    std::string user = "user1";
    std::string password = "password1";
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            print_help();
            return 0;
        } else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
            ip = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "-u") == 0 && i + 1 < argc) {
            user = argv[++i];
        } else if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            password = argv[++i];
        }
    }
    
    try {
        VectorTestClient client(ip, port);
        
        std::cout << "Connecting to " << ip << ":" << port << "..." << std::endl;
        if (!client.connect()) {
            std::cerr << "Connection failed" << std::endl;
            return 1;
        }
        
        std::cout << "Connected to server" << std::endl;
        
        if (!client.authenticate(user, password)) {
            return 1;
        }
        
        std::vector<std::vector<int32_t>> test_vectors = {
            {1, 2, 3, 4},
            {5, 6},
            {10, 20, 30},
            {2, 3, 5, 7, 11},
            {1000, 1000, 1000},
            {-1, 2, -3, 4},
            {0, 5, 10},
            {2147483647, 2}
        };
        
        std::cout << "\nSending " << test_vectors.size() << " vectors for multiplication..." << std::endl;
        client.send_vectors(test_vectors);
        
        std::cout << "Receiving results..." << std::endl;
        std::vector<int32_t> results = client.receive_results();
        
        std::cout << "\n=== RESULTS ===" << std::endl;
        for (size_t i = 0; i < results.size(); i++) {
            std::cout << "Vector " << (i + 1) << ": [";
            for (size_t j = 0; j < test_vectors[i].size(); j++) {
                std::cout << test_vectors[i][j];
                if (j < test_vectors[i].size() - 1) std::cout << ", ";
            }
            std::cout << "] = " << results[i] << std::endl;
        }
        
        client.close();
        std::cout << "\nTest completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
