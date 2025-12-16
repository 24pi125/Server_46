#include "../include/types.h"
#include <UnitTest++/UnitTest++.h>
#include <vector>
#include <cstdint>

SUITE(TypesTest) {
    TEST(VectorType) {
        Vector vec;
        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);
        
        CHECK_EQUAL(3, vec.size());
        CHECK_EQUAL(1, vec[0]);
        CHECK_EQUAL(2, vec[1]);
        CHECK_EQUAL(3, vec[2]);
    }
    
    TEST(ByteArrayType) {
        ByteArray bytes;
        bytes.push_back(0x00);
        bytes.push_back(0xFF);
        bytes.push_back(0x80);
        bytes.push_back(0x7F);
        
        CHECK_EQUAL(4, bytes.size());
        CHECK_EQUAL(0x00, bytes[0]);
        CHECK_EQUAL(0xFF, bytes[1]);
        CHECK_EQUAL(0x80, bytes[2]);
        CHECK_EQUAL(0x7F, bytes[3]);
    }
    
    TEST(TypeAliasesCompatibility) {
        std::vector<int32_t> std_vec = {1, 2, 3};
        Vector alias_vec = std_vec;
        
        CHECK_EQUAL(std_vec.size(), alias_vec.size());
        for (size_t i = 0; i < std_vec.size(); i++) {
            CHECK_EQUAL(std_vec[i], alias_vec[i]);
        }
    }
}

int main() {
    return UnitTest::RunAllTests();
}
