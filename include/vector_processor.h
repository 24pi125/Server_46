#ifndef VECTOR_PROCESSOR_H
#define VECTOR_PROCESSOR_H

#include "types.h"
#include <cstdint>
#include <vector>

class VectorProcessor {
public:
    static int32_t calculate_product(const Vector& vector);
    static std::vector<int32_t> multiply_vectors(const std::vector<Vector>& vectors);
};

#endif
