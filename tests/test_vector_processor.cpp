#include "../include/vector_processor.h"
#include <UnitTest++/UnitTest++.h>
#include <vector>
#include <climits>
#include <iostream>

SUITE(VectorProcessorTest) {
    TEST(EmptyVector) {
        std::vector<int32_t> empty;
        CHECK_EQUAL(0, VectorProcessor::calculate_product(empty));
    }
    
    TEST(SingleElement) {
        std::vector<int32_t> vec = {5};
        CHECK_EQUAL(5, VectorProcessor::calculate_product(vec));
    }
    
    TEST(MultipleElements) {
        std::vector<int32_t> vec = {2, 3, 4};
        CHECK_EQUAL(24, VectorProcessor::calculate_product(vec));
    }
    
    TEST(NegativeElements) {
        std::vector<int32_t> vec = {-2, 3, -4};
        CHECK_EQUAL(24, VectorProcessor::calculate_product(vec));
    }
    
    TEST(ZeroInVector) {
        std::vector<int32_t> vec = {2, 0, 4};
        CHECK_EQUAL(0, VectorProcessor::calculate_product(vec));
    }
    
    TEST(AllZeros) {
        std::vector<int32_t> vec = {0, 0, 0};
        CHECK_EQUAL(0, VectorProcessor::calculate_product(vec));
    }
    
    TEST(OverflowPositive) {
        std::vector<int32_t> vec = {INT_MAX, 2};
        CHECK_EQUAL(INT_MAX, VectorProcessor::calculate_product(vec));
    }
    
    TEST(OverflowNegative) {
        std::vector<int32_t> vec = {INT_MIN, 2};
        CHECK_EQUAL(INT_MIN, VectorProcessor::calculate_product(vec));
    }
    
    TEST(MultipleVectors) {
        std::vector<std::vector<int32_t>> vectors = {
            {1, 2, 3},
            {4, 5},
            {-1, -2, -3}
        };
        
        auto results = VectorProcessor::multiply_vectors(vectors);
        CHECK_EQUAL(3, results.size());
        CHECK_EQUAL(6, results[0]);
        CHECK_EQUAL(20, results[1]);
        CHECK_EQUAL(-6, results[2]);
    }
}

int main() {
    return UnitTest::RunAllTests();
}
