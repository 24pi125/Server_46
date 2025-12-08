#include "../include/auth.h"
#include <openssl/md5.h>
#include <sstream>
#include <iomanip>
#include <random>

std::string Authenticator::generate_salt_16() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    
    uint64_t salt = dis(gen);
    
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << salt;
    return ss.str();
}

std::string Authenticator::calculate_md5_hash(const std::string& salt, const std::string& password) {
    std::string data = salt + password;
    
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), hash);
    
    std::stringstream ss;
    ss << std::hex << std::uppercase;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        ss << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

bool Authenticator::verify_client(const std::string& login, const std::string& received_hash, 
                                const std::string& salt, 
                                const std::unordered_map<std::string, std::string>& clients) {
    auto it = clients.find(login);
    if (it == clients.end()) {
        return false;
    }
    
    std::string calculated_hash = calculate_md5_hash(salt, it->second);
    return calculated_hash == received_hash;
}
