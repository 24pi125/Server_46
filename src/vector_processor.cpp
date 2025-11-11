#include "../include/vector_processor.h"
#include <climits>
#include <stdexcept>
#include <iostream>

int32_t VectorProcessor::calculate_product(const Vector& vector) {
    if (vector.empty()) {
        return 0;
    }
    
    int64_t product = 1;
    
    for (int32_t value : vector) {
        product *= static_cast<int64_t>(value);
        
        // Check for overflow - согласно ТЗ при переполнении вверх возвращать 2^31-1
        if (product > 2147483647LL) {
            return 2147483647;
        }
        if (product < -2147483648LL) {
            return -2147483648;
        }
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
