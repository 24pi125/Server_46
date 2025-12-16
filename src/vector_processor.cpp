#include "../include/vector_processor.h"
#include <climits>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

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

std::vector<int32_t> VectorProcessor::multiply_vectors(const std::vector<Vector>& vectors) {
    std::vector<int32_t> results;
    results.reserve(vectors.size());
    
    for (const auto& vector : vectors) {
        results.push_back(calculate_product(vector));
    }
    
    return results;
}
