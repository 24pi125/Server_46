#include "../include/session.h"
#include "../include/auth.h"
#include "../include/vector_processor.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

Session::Session(int client_socket, std::unordered_map<std::string, std::string>& clients, Logger& logger)
    : client_socket_(client_socket), clients_(clients), logger_(logger) {}

void Session::handle() {
    try {
        if (!authenticate()) {
            close(client_socket_);
            return;
        }
        
        process_vectors();
        
    } catch (const std::exception& e) {
        logger_.log_error(std::string("Session error: ") + e.what(), false);
    }
    
    close(client_socket_);
}

bool Session::authenticate() {
    // Получаем логин
    char login_buf[256] = {0};
    ssize_t bytes = recv(client_socket_, login_buf, 255, 0);
    if (bytes <= 0) {
        logger_.log_error("Failed to receive login", false);
        return false;
    }
    std::string login(login_buf);
    login.erase(login.find_last_not_of(" \n\r\t") + 1);
    
    logger_.log("Received login: " + login);
    
    // Проверяем логин (только user)
    if (login != "user") {
        const char* err_msg = "ERR";
        send(client_socket_, err_msg, strlen(err_msg), 0);
        logger_.log_error("Invalid login: " + login, false);
        return false;
    }
    
    // Отправляем соль (64 бита -> 16 hex символов)
    std::string salt = Authenticator::generate_salt();
    logger_.log("Generated salt: " + salt);
    send(client_socket_, salt.c_str(), salt.length(), 0);
    
    // Получаем хэш
    char hash_buf[33] = {0};
    bytes = recv(client_socket_, hash_buf, 32, 0);
    if (bytes <= 0) {
        logger_.log_error("Failed to receive hash", false);
        return false;
    }
    std::string hash(hash_buf);
    hash.erase(hash.find_last_not_of(" \n\r\t") + 1);
    
    logger_.log("Received hash: " + hash);
    
    // Проверяем аутентификацию
    if (Authenticator::verify_client(login, hash, salt, clients_)) {
        const char* ok_msg = "OK";
        send(client_socket_, ok_msg, strlen(ok_msg), 0);
        logger_.log("Authentication successful");
        return true;
    } else {
        const char* err_msg = "ERR";
        send(client_socket_, err_msg, strlen(err_msg), 0);
        logger_.log_error("Authentication failed", false);
        return false;
    }
}

void Session::process_vectors() {
    uint32_t vector_count = receive_uint32();
    vector_count = ntohl(vector_count);
    
    logger_.log("Processing " + std::to_string(vector_count) + " vectors");
    
    std::vector<Vector> vectors;
    vectors.reserve(vector_count);
    
    for (uint32_t i = 0; i < vector_count; i++) {
        uint32_t vector_size = receive_uint32();
        vector_size = ntohl(vector_size);
        
        Vector vector = receive_vector(vector_size);
        vectors.push_back(vector);
        
        logger_.log("Received vector " + std::to_string(i + 1) + " with size " + std::to_string(vector_size));
    }
    
    std::vector<int32_t> results = VectorProcessor::multiply_vectors(vectors);
    send_results(results);
    
    logger_.log("Sent " + std::to_string(results.size()) + " results to client");
}

uint32_t Session::receive_uint32() {
    uint32_t value;
    ssize_t bytes_received = recv(client_socket_, &value, sizeof(value), 0);
    if (bytes_received != sizeof(value)) {
        throw std::runtime_error("Failed to receive uint32");
    }
    return value;
}

Vector Session::receive_vector(uint32_t size) {
    Vector vector(size);
    ssize_t bytes_received = recv(client_socket_, vector.data(), size * sizeof(int32_t), 0);
    if (bytes_received != static_cast<ssize_t>(size * sizeof(int32_t))) {
        throw std::runtime_error("Failed to receive vector data");
    }
    
    for (auto& val : vector) {
        val = ntohl(val);
    }
    
    return vector;
}

void Session::send_uint32(uint32_t value) {
    value = htonl(value);
    send(client_socket_, &value, sizeof(value), 0);
}

void Session::send_int32(int32_t value) {
    value = htonl(value);
    send(client_socket_, &value, sizeof(value), 0);
}

void Session::send_results(const std::vector<int32_t>& results) {
    send_uint32(static_cast<uint32_t>(results.size()));
    
    for (int32_t result : results) {
        send_int32(result);
    }
}
