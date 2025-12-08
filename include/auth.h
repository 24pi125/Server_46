#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <unordered_map>

class Authenticator {
public:
    static std::string generate_salt_16(); // 64 бита -> 16 hex символов
    static std::string calculate_md5_hash(const std::string& salt, const std::string& password);
    static bool verify_client(const std::string& login, const std::string& received_hash, 
                            const std::string& salt, 
                            const std::unordered_map<std::string, std::string>& clients);
};

#endif
