#include "../include/auth.h"
#include <UnitTest++/UnitTest++.h>
#include <unordered_map>
#include <iostream>
#include <cstring>

SUITE(AuthenticatorTest) {
    TEST(SaltGenerationLength) {
        std::string salt = Authenticator::generate_salt_16();
        CHECK_EQUAL(16, salt.length());
        
        bool all_hex = true;
        for (char c : salt) {
            if (!isxdigit(c)) {
                all_hex = false;
                break;
            }
        }
        CHECK(all_hex);
    }
    
    TEST(SaltUniqueness) {
        std::string salt1 = Authenticator::generate_salt_16();
        std::string salt2 = Authenticator::generate_salt_16();
        CHECK(salt1 != salt2);
    }
    
    TEST(MD5HashCalculation) {
        std::string salt = "4F9C429F5C6884DB";
        std::string password = "testpass123";
        
        std::string hash = Authenticator::calculate_md5_hash(salt, password);
        CHECK_EQUAL(32, hash.length());
        
        bool all_hex = true;
        for (char c : hash) {
            if (!isxdigit(c)) {
                all_hex = false;
                break;
            }
        }
        CHECK(all_hex);
    }
    
    TEST(VerifyClientSuccess) {
        std::unordered_map<std::string, std::string> clients = {
            {"user1", "pass1"},
            {"user2", "pass2"}
        };
        
        std::string salt = "4F9C429F5C6884DB";
        std::string password = "pass1";
        
        std::string calculated_hash = Authenticator::calculate_md5_hash(salt, password);
        
        bool result = Authenticator::verify_client("user1", calculated_hash, salt, clients);
        CHECK(result);
    }
    
    TEST(VerifyClientFailure) {
        std::unordered_map<std::string, std::string> clients = {
            {"user1", "pass1"}
        };
        
        std::string salt = "4F9C429F5C6884DB";
        std::string wrong_hash = "1234567890ABCDEF1234567890ABCDEF";
        
        bool result = Authenticator::verify_client("user1", wrong_hash, salt, clients);
        CHECK(!result);
    }
    
    TEST(VerifyClientNotFound) {
        std::unordered_map<std::string, std::string> clients = {
            {"user1", "pass1"}
        };
        
        std::string salt = "4F9C429F5C6884DB";
        std::string hash = "1234567890ABCDEF1234567890ABCDEF";
        
        bool result = Authenticator::verify_client("nonexistent", hash, salt, clients);
        CHECK(!result);
    }
}

int main() {
    return UnitTest::RunAllTests();
}
