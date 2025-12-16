#include <UnitTest++/UnitTest++.h>
#include <string>
#include <vector>
#include <climits>
#include <algorithm>

// Тестируемые интерфейсные функции
namespace InterfaceTest {
    // Интерфейс извлечения данных из буфера
    std::string extract_data_interface(const std::string& buffer, size_t& pos, size_t length) {
        if (pos + length > buffer.length()) {
            throw std::runtime_error("Not enough data in buffer");
        }
        
        std::string result = buffer.substr(pos, length);
        pos += length;
        return result;
    }
    
    // Интерфейс проверки hex строки
    bool is_valid_hex_string(const std::string& str) {
        if (str.empty()) {
            return false;  // Пустая строка не является валидной hex-строкой
        }
        
        for (char c : str) {
            if (!isxdigit(c)) {
                return false;
            }
        }
        return true;
    }
    
    // Интерфейс проверки длины хэша
    bool is_valid_hash_length(const std::string& hash) {
        return hash.length() == 32;
    }
    
    // Интерфейс проверки длины соли
    bool is_valid_salt_length(const std::string& salt) {
        return salt.length() == 16;
    }
    
    // Интерфейс проверки формата аутентификационной строки
    bool is_valid_auth_format(const std::string& login, 
                              const std::string& salt, 
                              const std::string& hash) {
        return !login.empty() && 
               is_valid_salt_length(salt) && 
               is_valid_hash_length(hash) &&
               is_valid_hex_string(salt) &&
               is_valid_hex_string(hash);
    }
    
    // Интерфейс расчета произведения вектора (упрощенный)
    int32_t calculate_product_interface(const std::vector<int32_t>& vector) {
        if (vector.empty()) {
            return 0;
        }
        
        int64_t product = 1;
        for (int32_t val : vector) {
            int64_t val64 = static_cast<int64_t>(val);
            
            // Упрощенная проверка переполнения
            if (val64 == 0) {
                return 0;
            }
            
            product *= val64;
            
            if (product > INT32_MAX) return INT32_MAX;
            if (product < INT32_MIN) return INT32_MIN;
        }
        
        return static_cast<int32_t>(product);
    }
}

SUITE(InterfaceTestSuite) {
    // ==================== ТЕСТЫ ИЗВЛЕЧЕНИЯ ДАННЫХ ====================
    TEST(ExtractDataBasic) {
        std::string buffer = "test1234567890ABCDEF";
        size_t pos = 0;
        
        std::string part1 = InterfaceTest::extract_data_interface(buffer, pos, 4);
        CHECK_EQUAL("test", part1);
        CHECK_EQUAL(4, pos);
        
        std::string part2 = InterfaceTest::extract_data_interface(buffer, pos, 6);
        CHECK_EQUAL("123456", part2);
        CHECK_EQUAL(10, pos);
        
        std::string part3 = InterfaceTest::extract_data_interface(buffer, pos, 10);
        CHECK_EQUAL("7890ABCDEF", part3);
        CHECK_EQUAL(20, pos);
    }
    
    TEST(ExtractDataComplete) {
        std::string buffer = "full_buffer_content";
        size_t pos = 0;
        
        std::string full = InterfaceTest::extract_data_interface(buffer, pos, buffer.length());
        CHECK_EQUAL(buffer, full);
        CHECK_EQUAL(buffer.length(), pos);
    }
    
    TEST(ExtractDataEmptyBuffer) {
        std::string buffer = "";
        size_t pos = 0;
        
        CHECK_THROW(InterfaceTest::extract_data_interface(buffer, pos, 1), std::runtime_error);
    }
    
    TEST(ExtractDataOutOfBounds) {
        std::string buffer = "short";
        size_t pos = 0;
        
        InterfaceTest::extract_data_interface(buffer, pos, 3); // "sho"
        
        CHECK_THROW(InterfaceTest::extract_data_interface(buffer, pos, 10), std::runtime_error);
    }
    
    // ==================== ТЕСТЫ ВАЛИДАЦИИ HEX ====================
    TEST(HexValidationPositive) {
        CHECK(InterfaceTest::is_valid_hex_string("1234567890"));
        CHECK(InterfaceTest::is_valid_hex_string("ABCDEF"));
        CHECK(InterfaceTest::is_valid_hex_string("abcdef"));
        CHECK(InterfaceTest::is_valid_hex_string("1234567890ABCDEFabcdef"));
    }
    
    TEST(HexValidationNegative) {
        CHECK(!InterfaceTest::is_valid_hex_string("")); // пустая
        CHECK(!InterfaceTest::is_valid_hex_string("123G456")); // G не hex
        CHECK(!InterfaceTest::is_valid_hex_string("12 34")); // пробел
        CHECK(!InterfaceTest::is_valid_hex_string("12@34")); // @ не hex
        CHECK(!InterfaceTest::is_valid_hex_string("12\n34")); // перевод строки
    }
    
    TEST(HexValidationMixedCase) {
        CHECK(InterfaceTest::is_valid_hex_string("12ABcdEF90"));
        CHECK(InterfaceTest::is_valid_hex_string("AaBbCcDdEeFf"));
    }
    
    // ==================== ТЕСТЫ ВАЛИДАЦИИ ДЛИНЫ ====================
    TEST(LengthValidationHash) {
        CHECK(InterfaceTest::is_valid_hash_length("12345678901234567890123456789012")); // 32 символа
        CHECK(!InterfaceTest::is_valid_hash_length("1234567890123456789012345678901")); // 31 символ
        CHECK(!InterfaceTest::is_valid_hash_length("123456789012345678901234567890123")); // 33 символа
        CHECK(!InterfaceTest::is_valid_hash_length("")); // пустая
    }
    
    TEST(LengthValidationSalt) {
        CHECK(InterfaceTest::is_valid_salt_length("1234567890123456")); // 16 символов
        CHECK(!InterfaceTest::is_valid_salt_length("123456789012345")); // 15 символов
        CHECK(!InterfaceTest::is_valid_salt_length("12345678901234567")); // 17 символов
    }
    
    // ==================== ТЕСТЫ ФОРМАТА АУТЕНТИФИКАЦИИ ====================
    TEST(AuthFormatValid) {
        CHECK(InterfaceTest::is_valid_auth_format(
            "user", 
            "1234567890ABCDEF", 
            "098F6BCD4621D373CADE4E832627B4F6"
        ));
    }
    
    TEST(AuthFormatInvalidLogin) {
        CHECK(!InterfaceTest::is_valid_auth_format(
            "", // пустой логин
            "1234567890ABCDEF", 
            "098F6BCD4621D373CADE4E832627B4F6"
        ));
    }
    
    TEST(AuthFormatInvalidSaltLength) {
        CHECK(!InterfaceTest::is_valid_auth_format(
            "user",
            "1234567890ABCDE", // 15 символов
            "098F6BCD4621D373CADE4E832627B4F6"
        ));
    }
    
    TEST(AuthFormatInvalidHashLength) {
        CHECK(!InterfaceTest::is_valid_auth_format(
            "user",
            "1234567890ABCDEF",
            "098F6BCD4621D373CADE4E832627B4F" // 31 символ
        ));
    }
    
    TEST(AuthFormatInvalidHex) {
        CHECK(!InterfaceTest::is_valid_auth_format(
            "user",
            "1234567890ABCDEG", // G не hex
            "098F6BCD4621D373CADE4E832627B4F6"
        ));
        
        CHECK(!InterfaceTest::is_valid_auth_format(
            "user",
            "1234567890ABCDEF",
            "098F6BCD4621D373CADE4E832627B4G6" // G не hex
        ));
    }
    
    // ==================== ТЕСТЫ ВЫЧИСЛЕНИЯ ПРОИЗВЕДЕНИЯ ====================
    TEST(ProductCalculationBasic) {
        std::vector<int32_t> vec1 = {2, 3, 4};
        CHECK_EQUAL(24, InterfaceTest::calculate_product_interface(vec1));
        
        std::vector<int32_t> vec2 = {5};
        CHECK_EQUAL(5, InterfaceTest::calculate_product_interface(vec2));
        
        std::vector<int32_t> vec3 = {-2, 3, -4};
        CHECK_EQUAL(24, InterfaceTest::calculate_product_interface(vec3));
    }
    
    TEST(ProductCalculationEdgeCases) {
        std::vector<int32_t> empty;
        CHECK_EQUAL(0, InterfaceTest::calculate_product_interface(empty));
        
        std::vector<int32_t> with_zero = {1, 0, 3};
        CHECK_EQUAL(0, InterfaceTest::calculate_product_interface(with_zero));
        
        std::vector<int32_t> all_zeros = {0, 0, 0};
        CHECK_EQUAL(0, InterfaceTest::calculate_product_interface(all_zeros));
    }
    
    TEST(ProductCalculationLargeNumbers) {
        std::vector<int32_t> vec = {1000, 2000};
        CHECK_EQUAL(2000000, InterfaceTest::calculate_product_interface(vec));
    }
    
    TEST(ProductCalculationOverflowProtection) {
        std::vector<int32_t> vec = {INT_MAX, 2};
        int32_t result = InterfaceTest::calculate_product_interface(vec);
        CHECK(result == INT_MAX);
        
        std::vector<int32_t> vec2 = {INT_MIN, 2};
        int32_t result2 = InterfaceTest::calculate_product_interface(vec2);
        CHECK(result2 == INT_MIN || result2 == INT_MAX);
    }
    
    // ==================== ТЕСТЫ ИНТЕГРАЦИИ ИНТЕРФЕЙСОВ ====================
    TEST(InterfaceIntegration) {
        // Тест полного цикла обработки аутентификационной строки
        std::string auth_data = "john_doe4F9C429F5C6884DBD41D8CD98F00B204E9800998ECF8427E";
        size_t pos = 0;
        
        // Извлекаем логин (8 символов)
        std::string login = InterfaceTest::extract_data_interface(auth_data, pos, 8);
        CHECK_EQUAL("john_doe", login);
        
        // Извлекаем соль (16 символов)
        std::string salt = InterfaceTest::extract_data_interface(auth_data, pos, 16);
        CHECK_EQUAL("4F9C429F5C6884DB", salt);
        
        // Извлекаем хэш (32 символа)
        std::string hash = InterfaceTest::extract_data_interface(auth_data, pos, 32);
        CHECK_EQUAL("D41D8CD98F00B204E9800998ECF8427E", hash);
        
        // Проверяем формат
        CHECK(InterfaceTest::is_valid_auth_format(login, salt, hash));
        
        // Проверяем отдельные компоненты
        CHECK(InterfaceTest::is_valid_salt_length(salt));
        CHECK(InterfaceTest::is_valid_hash_length(hash));
        CHECK(InterfaceTest::is_valid_hex_string(salt));
        CHECK(InterfaceTest::is_valid_hex_string(hash));
    }
    
    TEST(InterfaceErrorHandling) {
        // Тест обработки ошибок в интерфейсе
        // Увеличиваем длину строки для извлечения всех данных
        std::string invalid_auth = "user123G4567890ABCD1234567890ABCDEF1234567890ABCDEF12345678";
        size_t pos = 0;
        
        std::string login = InterfaceTest::extract_data_interface(invalid_auth, pos, 4);
        std::string salt = InterfaceTest::extract_data_interface(invalid_auth, pos, 16);
        std::string hash = InterfaceTest::extract_data_interface(invalid_auth, pos, 32);
        
        // Соль содержит не-hex символ 'G'
        CHECK(!InterfaceTest::is_valid_hex_string(salt));
        CHECK(!InterfaceTest::is_valid_auth_format(login, salt, hash));
    }
}

int main() {
    return UnitTest::RunAllTests();
}
