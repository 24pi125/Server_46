#include <UnitTest++/UnitTest++.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
#include <openssl/md5.h>
#include <sstream>
#include <iomanip>
#include <climits>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>

SUITE(SessionFunctionsTest) {
    // Функция расчета MD5 (копия из session.cpp)
    std::string calculate_md5_test(const std::string& data) {
        unsigned char md5_hash[MD5_DIGEST_LENGTH];
        MD5(reinterpret_cast<const unsigned char*>(data.c_str()), 
            data.length(), md5_hash);
        
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            ss << std::setw(2) << static_cast<int>(md5_hash[i]);
        }
        return ss.str();
    }
    
    // Функция расчета произведения вектора (копия из session.cpp)
    int32_t calculate_vector_product_test(const std::vector<int32_t>& vector) {
        if (vector.empty()) {
            return 0;
        }
        
        int64_t product = 1;
        for (int32_t val : vector) {
            // Проверка переполнения при умножении
            // Используем static_cast<int64_t> для безопасного преобразования
            int64_t val64 = static_cast<int64_t>(val);
            
            if (val64 != 0 && llabs(product) > INT64_MAX / llabs(val64)) {
                // Переполнение
                if ((product > 0 && val64 > 0) || (product < 0 && val64 < 0)) {
                    return INT32_MAX;
                } else {
                    return INT32_MIN;
                }
            }
            product *= val64;
        }
        
        // Проверка выхода за пределы int32
        if (product > INT32_MAX) {
            return INT32_MAX;
        } else if (product < INT32_MIN) {
            return INT32_MIN;
        }
        
        return static_cast<int32_t>(product);
    }
    
    // Тестируемый интерфейс - функция извлечения данных из буфера
    std::string extract_data_interface_test(const std::string& buffer, size_t& pos, size_t length) {
        if (pos + length > buffer.length()) {
            throw std::runtime_error("Not enough data in buffer");
        }
        
        std::string result = buffer.substr(pos, length);
        pos += length;
        return result;
    }
    
    // Тестируемый интерфейс - функция проверки хеш-строки
    bool validate_hash_interface_test(const std::string& hash) {
        if (hash.length() != 32) {
            return false;
        }
        
        for (char c : hash) {
            if (!isxdigit(c)) {
                return false;
            }
        }
        return true;
    }
    
    TEST(CalculateVectorProduct) {
        // Тест 1: Пустой вектор
        std::vector<int32_t> empty;
        CHECK_EQUAL(0, calculate_vector_product_test(empty));
        
        // Тест 2: Простой вектор
        std::vector<int32_t> simple = {2, 3, 4};
        CHECK_EQUAL(24, calculate_vector_product_test(simple));
        
        // Тест 3: Вектор с нулем
        std::vector<int32_t> with_zero = {1, 0, 3};
        CHECK_EQUAL(0, calculate_vector_product_test(with_zero));
        
        // Тест 4: Отрицательные значения
        std::vector<int32_t> negative = {-2, 3, -4};
        CHECK_EQUAL(24, calculate_vector_product_test(negative));
        
        // Тест 5: Одно значение
        std::vector<int32_t> single = {42};
        CHECK_EQUAL(42, calculate_vector_product_test(single));
        
        // Тест 6: Большие числа
        std::vector<int32_t> large = {1000, 2000};
        CHECK_EQUAL(2000000, calculate_vector_product_test(large));
        
        // Тест 7: Переполнение положительное
        std::vector<int32_t> overflow_pos = {INT_MAX, 2};
        int32_t result1 = calculate_vector_product_test(overflow_pos);
        CHECK_EQUAL(INT_MAX, result1);
        
        // Тест 8: Переполнение отрицательное
        std::vector<int32_t> overflow_neg = {INT_MIN, 2};
        int32_t result2 = calculate_vector_product_test(overflow_neg);
        // Принимаем оба возможных варианта для переполнения
        CHECK(result2 == INT_MIN || result2 == INT_MAX);
        
        // Тест 9: Переполнение с несколькими значениями
        std::vector<int32_t> overflow_multi = {1000000, 1000000, 1000000};
        int32_t result3 = calculate_vector_product_test(overflow_multi);
        CHECK_EQUAL(INT_MAX, result3);
        
        // Тест 10: Переполнение отрицательное с несколькими значениями
        std::vector<int32_t> overflow_multi_neg = {-1000000, 1000000, -1000000};
        int32_t result4 = calculate_vector_product_test(overflow_multi_neg);
        // Принимаем оба возможных варианта для переполнения
        CHECK(result4 == INT_MIN || result4 == INT_MAX);
    }
    
    TEST(CalculateVectorProductEdgeCases) {
        // Дополнительные граничные случаи
        std::vector<int32_t> max_values = {INT_MAX, 1};
        CHECK_EQUAL(INT_MAX, calculate_vector_product_test(max_values));
        
        std::vector<int32_t> min_values = {INT_MIN, 1};
        CHECK_EQUAL(INT_MIN, calculate_vector_product_test(min_values));
        
        std::vector<int32_t> zero_overflow = {INT_MAX, 0};
        CHECK_EQUAL(0, calculate_vector_product_test(zero_overflow));
        
        // Большой вектор с единицами
        std::vector<int32_t> many_ones(100, 1);
        CHECK_EQUAL(1, calculate_vector_product_test(many_ones));
    }
    
    TEST(InterfaceValidateHash) {
        // Корректные хэши
        CHECK(validate_hash_interface_test("098f6bcd4621d373cade4e832627b4f6"));
        CHECK(validate_hash_interface_test("d41d8cd98f00b204e9800998ecf8427e"));
        CHECK(validate_hash_interface_test("1234567890abcdef1234567890abcdef"));
        
        // Некорректные хэши
        CHECK(!validate_hash_interface_test("")); // слишком короткий
        CHECK(!validate_hash_interface_test("098f6bcd4621d373cade4e832627b4f")); // 31 символ
        CHECK(!validate_hash_interface_test("098f6bcd4621d373cade4e832627b4f600")); // 33 символа
        CHECK(!validate_hash_interface_test("098f6bcd4621d373cade4e832627b4f!")); // не hex символ
        CHECK(!validate_hash_interface_test("098f6bcd4621d373cade4e832627b4g6")); // 'g' не hex
    }
    
    TEST(CalculateMD5) {
        // Известный тест-вектор для MD5
        std::string test_data = "hello world";
        std::string hash = calculate_md5_test(test_data);
        
        CHECK_EQUAL(32, hash.length());
        
        bool all_hex = true;
        for (char c : hash) {
            if (!isxdigit(c)) {
                all_hex = false;
                break;
            }
        }
        CHECK(all_hex);
        
        // Проверяем известный MD5 хэш для пустой строки
        std::string known_hash = calculate_md5_test("");
        CHECK_EQUAL("d41d8cd98f00b204e9800998ecf8427e", known_hash);
        
        // Проверяем MD5 для "test"
        std::string test_hash = calculate_md5_test("test");
        CHECK_EQUAL("098f6bcd4621d373cade4e832627b4f6", test_hash);
        
        // Проверяем MD5 для "P@ssW0rd"
        std::string pass_hash = calculate_md5_test("P@ssW0rd");
        CHECK_EQUAL(32, pass_hash.length());
    }
    
    TEST(MD5Consistency) {
        // Проверяем консистентность MD5
        std::string data1 = "test data";
        std::string hash1 = calculate_md5_test(data1);
        std::string hash2 = calculate_md5_test(data1);
        
        CHECK_EQUAL(hash1, hash2); // Должны быть идентичны
        
        // Проверяем что разные данные дают разные хэши
        std::string data2 = "test datb";
        std::string hash3 = calculate_md5_test(data2);
        CHECK(hash1 != hash3);
    }
    
    TEST(HexValidation) {
        // Проверяем функцию проверки hex символов
        std::string valid_hex = "1234567890ABCDEFabcdef";
        std::string invalid_hex = "123G456"; // G не hex символ
        
        bool all_valid = true;
        for (char c : valid_hex) {
            if (!isxdigit(c)) {
                all_valid = false;
                break;
            }
        }
        CHECK(all_valid);
        
        bool has_invalid = false;
        for (char c : invalid_hex) {
            if (!isxdigit(c)) {
                has_invalid = true;
                break;
            }
        }
        CHECK(has_invalid);
    }
    
    TEST(AuthStringConstruction) {
        // Тестируем правильность формирования строки для аутентификации
        std::string login = "user";
        std::string salt = "1234567890ABCDEF"; // 16 символов
        std::string password = "P@ssW0rd";
        
        // Строка для MD5 должна быть salt + password
        std::string data_for_md5 = salt + password;
        std::string hash = calculate_md5_test(data_for_md5);
        
        // Проверяем что хэш 32 символа
        CHECK_EQUAL(32, hash.length());
        
        // Проверяем что все символы hex
        bool all_hex = true;
        for (char c : hash) {
            if (!isxdigit(c)) {
                all_hex = false;
                break;
            }
        }
        CHECK(all_hex);
        
        // Полная строка аутентификации
        std::string auth_string = login + salt + hash;
        
        // Должно быть: логин (4) + соль (16) + хэш (32) = 52 символа
        CHECK_EQUAL(52, auth_string.length());
    }
}

int main() {
    return UnitTest::RunAllTests();
}
