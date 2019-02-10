#include "distmat.h"
#include <random>

template<typename T>
void test_span(size_t n) {
    dm::DistanceMatrix<T> mat(n);
    std::mt19937_64 mt(n + sizeof(T) + std::is_integral<T>::value);
    std::gamma_distribution<double> gamrock(137);
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
        for(size_t k = 0; k < span.second; ++k)
            std::fprintf(stderr, "span v: %lf\n", double(span[k]));
        for(size_t k = 0; k < mat.size(); ++k)
            std::fprintf(stderr, "mat v: %lf\n", double(span[k + i + 1]));
        for(size_t j = 0; j < span.second; ++j) {
            assert(span.first[j] == mat(i, j + i + 1));
        }
    }
}

int main() {
    size_t n = 50;
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
    n = 10000;
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
