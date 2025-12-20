/**
 * @file vector_processor.cpp
 * @brief Реализация векторных вычислений
 * 
 * Содержит реализацию методов класса VectorProcessor:
 * - вычисление произведения элементов вектора с контролем переполнения
 * - пакетная обработка коллекции векторов
 * 
 * @see vector_processor.h
 */

#include "../include/vector_processor.h"
#include <climits>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

/**
 * @brief Вычисляет произведение элементов вектора
 * 
 * @param vector Входной вектор целых чисел
 * @return int32_t Произведение элементов или граничное значение при переполнении
 * 
 * @details
 * Алгоритм:
 * 1. Проверка пустого вектора -> возврат 0
 * 2. Инициализация 64-битного аккумулятора значением 1
 * 3. Для каждого элемента:
 *    - Проверка переполнения перед умножением
 *    - Если переполнение -> возврат INT32_MAX или INT32_MIN
 *    - Иначе умножение аккумулятора на элемент
 * 4. Проверка результата на соответствие 32-битному диапазону
 * 
 * @example
 * Vector v = {1, 2, 3};
 * int32_t result = VectorProcessor::calculate_product(v); // 6
 * 
 * Vector large = {INT32_MAX, 2};
 * int32_t overflow = VectorProcessor::calculate_product(large); // INT32_MAX
 */
int32_t VectorProcessor::calculate_product(const Vector& vector) {
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
    }
    if (product < INT32_MIN) {
        return INT32_MIN;
    }
    
    return static_cast<int32_t>(product);
}

/**
 * @brief Вычисляет произведения для коллекции векторов
 * 
 * @param vectors Коллекция векторов для обработки
 * @return std::vector<int32_t> Вектор произведений для каждого входного вектора
 * 
 * @details
 * Обрабатывает векторы последовательно, применяя calculate_product() к каждому.
 * Размер выходного вектора равен количеству входных векторов.
 * 
 * @note Не модифицирует входные векторы
 * @note Возвращает пустой вектор если входная коллекция пуста
 */
std::vector<int32_t> VectorProcessor::multiply_vectors(const std::vector<Vector>& vectors) {
    std::vector<int32_t> results;
    results.reserve(vectors.size());
    
    for (const auto& vector : vectors) {
        results.push_back(calculate_product(vector));
    }
    
    return results;
}
