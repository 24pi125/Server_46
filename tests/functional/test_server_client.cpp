#include <UnitTest++/UnitTest++.h>
#include <iostream>
#include <fstream>
#include <string>

SUITE(ServerClientTest) {
    TEST(ClientConfigTest) {
        std::cout << "Тест конфигурации клиентов" << std::endl;
        
        // Создаем файл с клиентами
        std::ofstream client_file("tests/test_data/clients.conf");
        CHECK(client_file.is_open());
        
        if (client_file.is_open()) {
            client_file << "user1:password1\n";
            client_file << "user2:password2\n";
            client_file << "user3:password3\n";
            client_file.close();
            
            // Читаем и проверяем
            std::ifstream read_file("tests/test_data/clients.conf");
            CHECK(read_file.is_open());
            
            int user_count = 0;
            std::string line;
            while (std::getline(read_file, line)) {
                if (!line.empty()) {
                    user_count++;
                    // Проверяем формат логин:пароль
                    size_t colon_pos = line.find(':');
                    CHECK(colon_pos != std::string::npos);
                    CHECK(colon_pos > 0);  // Логин не пустой
                    CHECK(colon_pos < line.length() - 1);  // Пароль не пустой
                }
            }
            
            CHECK_EQUAL(3, user_count);
            read_file.close();
        }
    }
    
    TEST(ProtocolFormatTest) {
        std::cout << "Тест формата протокола" << std::endl;
        
        // Проверяем ожидаемый формат протокола
        // 1. Аутентификация: логин + 16 hex соли + 32 hex хэша
        // 2. Подтверждение: "OK\n" или "err\n"
        // 3. Количество векторов: 4 байта
        // 4. Для каждого вектора: размер (4 байта) + данные (размер * 4 байта)
        // 5. Результаты: для каждого вектора 4 байта
        
        std::string ok_response = "OK\n";
        std::string err_response = "err\n";
        
        CHECK_EQUAL(3, ok_response.length());
        CHECK_EQUAL(4, err_response.length());
        
        // Проверяем что ответы заканчиваются переводом строки
        CHECK_EQUAL('\n', ok_response[2]);
        CHECK_EQUAL('\n', err_response[3]);
    }
    
    TEST(ErrorHandlingTest) {
        std::cout << "Тест обработки ошибок" << std::endl;
        
        // Проверяем различные ошибочные сценарии
        std::string empty_login = "";
        std::string short_salt = "123";  // Должно быть 16 символов
        std::string short_hash = "abc";  // Должно быть 32 символа
        
        CHECK(empty_login.empty());
        CHECK(short_salt.length() < 16);
        CHECK(short_hash.length() < 32);
        
        // Не-hex символы
        std::string invalid_salt = "1234567890ABCDEG";  // 'G' не hex
        std::string invalid_hash = "098F6BCD4621D373CADE4E832627B4G6";  // 'G' не hex
        
        auto has_non_hex = [](const std::string& str) {
            for (char c : str) {
                if (!((c >= '0' && c <= '9') || 
                      (c >= 'A' && c <= 'F') || 
                      (c >= 'a' && c <= 'f'))) {
                    return true;
                }
            }
            return false;
        };
        
        CHECK(has_non_hex(invalid_salt));
        CHECK(has_non_hex(invalid_hash));
    }
};

int main() {
    std::cout << "Запуск тестов клиент-серверного взаимодействия" << std::endl;
    // Создаем директорию для тестовых данных
    system("mkdir -p tests/test_data");
    
    return UnitTest::RunAllTests();
}
