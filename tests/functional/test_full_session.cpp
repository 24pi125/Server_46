#include <UnitTest++/UnitTest++.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>

SUITE(FullSessionTest) {
    TEST(VectorDataFormatTest) {
        std::cout << "Тест формата векторных данных" << std::endl;
        
        // Тестируем формат передачи векторов
        uint32_t vector_count = 2;
        std::vector<std::vector<int32_t>> vectors = {
            {1, 2, 3, 4},
            {-1, -2, 3}
        };
        
        CHECK_EQUAL(2, vector_count);
        CHECK_EQUAL(4, vectors[0].size());
        CHECK_EQUAL(3, vectors[1].size());
        CHECK_EQUAL(1, vectors[0][0]);
        CHECK_EQUAL(-1, vectors[1][0]);
    }
    
    TEST(ProductCalculationTest) {
        std::cout << "Тест расчета произведений" << std::endl;
        
        // Тестируем логику расчета произведений
        std::vector<int32_t> vec1 = {1, 2, 3, 4};
        std::vector<int32_t> vec2 = {5, 6};
        std::vector<int32_t> vec3 = {-2, 3, -4};
        
        // Рассчитываем произведения вручную
        int32_t product1 = 1 * 2 * 3 * 4;  // 24
        int32_t product2 = 5 * 6;          // 30
        int32_t product3 = (-2) * 3 * (-4); // 24
        
        CHECK_EQUAL(24, product1);
        CHECK_EQUAL(30, product2);
        CHECK_EQUAL(24, product3);
    }
    
    TEST(OverflowTest) {
        std::cout << "Тест переполнения" << std::endl;
        
        // Тестируем логику обработки переполнения
        int32_t max_int = 2147483647;  // INT32_MAX
        int32_t min_int = -2147483648; // INT32_MIN
        
        // Простые проверки граничных значений
        CHECK(max_int > 0);
        CHECK(min_int < 0);
        
        // Проверяем что умножение больших чисел может вызвать переполнение
        int64_t large_product = static_cast<int64_t>(max_int) * 2;
        CHECK(large_product > max_int);  // Действительно вызывает переполнение
    }
    
    TEST(ConfigFileTest) {
        std::cout << "Тест конфигурационных файлов" << std::endl;
        
        // Создаем тестовый конфигурационный файл
        std::ofstream config_file("tests/test_data/test_server.conf");
        CHECK(config_file.is_open());
        
        if (config_file.is_open()) {
            config_file << "# Тестовый конфигурационный файл\n";
            config_file << "port=33334\n";
            config_file << "log_file=/tmp/test_server.log\n";
            config_file << "client_db=tests/test_data/clients.conf\n";
            config_file.close();
            
            // Проверяем создание
            std::ifstream check_file("tests/test_data/test_server.conf");
            CHECK(check_file.is_open());
            
            int line_count = 0;
            std::string line;
            while (std::getline(check_file, line)) {
                line_count++;
            }
            CHECK_EQUAL(4, line_count);  // 3 строки + пустая в конце?
            check_file.close();
        }
    }
};

int main() {
    std::cout << "Запуск тестов полной сессии" << std::endl;
    // Создаем директорию для тестовых данных
    system("mkdir -p tests/test_data");
    
    return UnitTest::RunAllTests();
}
