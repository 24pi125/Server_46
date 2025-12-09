#include <UnitTest++/UnitTest++.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
#include <openssl/md5.h>
#include <sstream>
#include <iomanip>
#include <climits>

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
            if (val != 0 && llabs(product) > INT64_MAX / llabs(val)) {
                // Переполнение
                if ((product > 0 && val > 0) || (product < 0 && val < 0)) {
                    return INT32_MAX;
                } else {
                    return INT32_MIN;
                }
            }
            product *= val;
        }
        
        // Проверка выхода за пределы int32
        if (product > INT32_MAX) {
            return INT32_MAX;
        } else if (product < INT32_MIN) {
            return INT32_MIN;
        }
        
        return static_cast<int32_t>(product);
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
        CHECK_EQUAL(INT_MAX, calculate_vector_product_test(overflow_pos));
        
        // Тест 8: Переполнение отрицательное
        std::vector<int32_t> overflow_neg = {INT_MIN, 2};
        CHECK_EQUAL(INT_MIN, calculate_vector_product_test(overflow_neg));
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
        
        // Должно быть: логин (4) + соль (16) + хэш (32) = 52+ символа
        CHECK(auth_string.length() >= 52);
    }
}

int main() {
    return UnitTest::RunAllTests();
}
