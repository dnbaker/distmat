#include "distmat.h"
#include <iostream>
#include <random>

template<typename T, bool use_mmap>
void test_span_impl(size_t n);
template<typename T>
void test_span(size_t n) {
    test_span_impl<T, false>(n);
    test_span_impl<T, true>(n);
}

template<typename T, bool use_mmap>
void test_span_impl(size_t n) {
    dm::DistanceMatrix<T, 0, static_cast<dm::MemoryStrategy>(use_mmap)> mat(n), mat2(n);
    std::mt19937_64 mt(n + sizeof(T) + std::is_integral<T>::value);
    std::gamma_distribution<double> gamrock(1);
    for(size_t i = 0; i < n; ++i) {
        for(size_t j = i + 1; j < n; ++j) {
            if(std::is_integral<T>::value) {
                mat(i, j) = mt();
            } else {
                mat(i, j) = gamrock(mt);
            }
        }
    }
    for(size_t i = 0; i < n; ++i) {
        auto span = mat.row_span(i);
        for(size_t j = 0; j < span.second; ++j) {
            assert(j + i + 1 < mat.size());
            //std::fprintf(stderr, "v1 %lf at row %zu, span %zu, expected index %zu, v2: %lf\n", span.first[j], i, j, j + i + 1, mat(i, j + i + 1));
            assert(span.first[j] == mat(i, j + i + 1));
        }
    }
}

int main() {
    size_t n = 10;
    test_span<double>(n);
    test_span<float>(n);
    test_span<uint8_t>(n);
    test_span<uint16_t>(n);
    test_span<uint32_t>(n);
    test_span<uint64_t>(n);
    test_span<int8_t>(n);
    test_span<int16_t>(n);
    test_span<int32_t>(n);
    test_span<int64_t>(n);
    dm::DistanceMatrix<float> mat(n), mat2(n);
    std::mt19937_64 mt(n + sizeof(float));
    std::gamma_distribution<double> gamrock(1);
    for(size_t i = 0; i < n; ++i) {
        for(size_t j = i + 1; j < n; ++j) {
            mat(i, j) = gamrock(mt);
        }
    }
    std::cerr << mat << '\n';
    n = 500;
    test_span<double>(n);
    test_span<float>(n);
    test_span<uint8_t>(n);
    test_span<uint16_t>(n);
    test_span<uint32_t>(n);
    test_span<uint64_t>(n);
    test_span<int8_t>(n);
    test_span<int16_t>(n);
    test_span<int32_t>(n);
    test_span<int64_t>(n);
}
