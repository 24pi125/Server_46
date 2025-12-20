#include <UnitTest++/UnitTest++.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

SUITE(NetworkAuthTest) {
    TEST(FileCreationTest) {
        std::cout << "Тест создания файлов для аутентификации" << std::endl;
        
        // Создаем тестовый файл клиентов
        std::ofstream client_file("tests/test_data/test_clients.conf");
        CHECK(client_file.is_open());
        
        if (client_file.is_open()) {
            client_file << "testuser:testpass\n";
            client_file << "alice:password123\n";
            client_file.close();
            CHECK(true);
        }
        
        // Проверяем что файл создан
        std::ifstream check_file("tests/test_data/test_clients.conf");
        CHECK(check_file.is_open());
        if (check_file.is_open()) {
            std::string line;
            std::getline(check_file, line);
            CHECK_EQUAL("testuser:testpass", line);
            check_file.close();
        }
    }
    
    TEST(AuthDataFormatTest) {
        std::cout << "Тест формата данных аутентификации" << std::endl;
        
        // Проверяем ожидаемый формат: логин + 16 hex соли + 32 hex хэша
        std::string login = "testuser";
        std::string salt = "1234567890ABCDEF";  // 16 hex символов
        std::string hash = "098F6BCD4621D373CADE4E832627B4F6";  // 32 hex символа
        
        CHECK_EQUAL(16, salt.length());
        CHECK_EQUAL(32, hash.length());
        
        // Проверяем что все символы hex
        auto is_hex_char = [](char c) {
            return (c >= '0' && c <= '9') || 
                   (c >= 'A' && c <= 'F') || 
                   (c >= 'a' && c <= 'f');
        };
        
        for (char c : salt) {
            CHECK(is_hex_char(c));
        }
        
        for (char c : hash) {
            CHECK(is_hex_char(c));
        }
        
        // Полная строка аутентификации
        std::string auth_string = login + salt + hash;
        CHECK_EQUAL(login.length() + 16 + 32, auth_string.length());
    }
};

int main() {
    std::cout << "Запуск тестов аутентификации" << std::endl;
    // Создаем директорию для тестовых данных
    system("mkdir -p tests/test_data");
    
    return UnitTest::RunAllTests();
}
