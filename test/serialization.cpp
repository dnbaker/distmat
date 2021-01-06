#include "distmat.h"
#include <iostream>
#include <random>

template<typename T>
void test_serialization(const size_t n) {
    {
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
        mat.write("tmpfile.txt.gz", 6);
        dm::DistanceMatrix<T> newmat("tmpfile.txt.gz", 0, 1., nullptr, true);
        assert(newmat == mat);
        dm::DistanceMatrix<T> onewmat("tmpfile.txt.new.gz", n, 1.);
        std::fprintf(stderr, "Created onewmat! %zu entries\n", onewmat.num_entries());
        std::fprintf(stderr, "mat nelem %zu. newmat %zu. onewmat: %zu\n", mat.nelem(), newmat.nelem(), onewmat.nelem());
        assert(onewmat.size() == newmat.size());
        std::memcpy(onewmat.data(), newmat.data(), sizeof(T) * onewmat.num_entries());
        std::fprintf(stderr, "Copied over\n", onewmat.num_entries());
        assert(onewmat == newmat);
        std::fprintf(stderr, "Passed assert\n", onewmat.num_entries());
    }
    if(std::system((std::string("rm ") + "tmpfile.txt.gz").data())) throw "a party";
    else if(std::system((std::string("rm ") + "tmpfile.txt.new.gz").data())) throw "a party";
    else if(access("tmpfile.txt.gz", F_OK) != -1 || access("tmpfile.txt.new.gz", F_OK) != -1)
         throw std::runtime_error("Failed to clean up after");
}

int main() {
    size_t n = 1000;
    test_serialization<double>(n);
    test_serialization<float>(n);
    test_serialization<uint8_t>(n);
    test_serialization<uint16_t>(n);
    test_serialization<uint32_t>(n);
    test_serialization<uint64_t>(n);
    test_serialization<int8_t>(n);
    test_serialization<int16_t>(n);
    test_serialization<int32_t>(n);
    test_serialization<int64_t>(n);
    std::fprintf(stderr, "Passed for n = 1000\n");
    n = 10000;
    test_serialization<double>(n);
    test_serialization<float>(n);
    test_serialization<uint8_t>(n);
    test_serialization<uint16_t>(n);
    test_serialization<uint32_t>(n);
    test_serialization<uint64_t>(n);
    test_serialization<int8_t>(n);
    test_serialization<int16_t>(n);
    test_serialization<int32_t>(n);
    test_serialization<int64_t>(n);
    std::fprintf(stderr, "Passed for n = 10000\n");
    std::FILE *tmp = std::fopen("zomg.file", "wb");
    std::fputc('\0', tmp);
    n = 1000; // go back to 1k by 1k for size reasons
    std::fwrite(&n, sizeof(n), 1, tmp);
    size_t nelem = (n * (n - 1)) >> 1;
    float val = 1.37;
    for(size_t i = 0; i < nelem; ++i)
        std::fwrite(&val, sizeof(val), 1, tmp);
    std::fclose(tmp);
    if(1) {
        dm::DistanceMatrix<float> dm("zomg.file");
        std::cout << dm;
    }
    ::system("rm zomg.file");
    //std::cerr << dm << '\n';
}
