#pragma once
#include <type_traits>
#include <cstring>
#include <vector>
#include <cassert>
#include <array>
#include <cstdlib>
#include <stdexcept>
#include <cstdint>
#include "unistd.h"

namespace dm {

namespace more_magic {
template<typename ArithType>
struct MAGIC_NUMBER {
    const char *name() {
        throw std::runtime_error("NotImplemented");
        return "NOTIMPLEMENTED";
    }
};

#define DEC_MAGIC(type, STR) \
    template<> struct MAGIC_NUMBER<type> {\
        static constexpr const char *name() {return "DM::" STR;}\
    }

DEC_MAGIC(float, "float");
DEC_MAGIC(double,"double");
DEC_MAGIC(uint8_t,"uint8_t");
DEC_MAGIC(uint16_t,"uint16_t");
DEC_MAGIC(uint32_t,"uint32_t");
DEC_MAGIC(uint64_t,"uint64_t");
DEC_MAGIC(int8_t,"int8_t");
DEC_MAGIC(int16_t,"int16_t");
DEC_MAGIC(int32_t,"int32_t");
DEC_MAGIC(int64_t,"int64_t");
} // namespace more_magic

#undef DEC_MAGIC
template<typename ArithType=float,
         size_t DefaultValue=0,
         typename=typename std::enable_if<std::is_arithmetic<ArithType>::value>::type
         >
class DistanceMatrix {
    std::vector<ArithType> data_;
    uint64_t  nelem_;
    ArithType default_value_;
public:
    static const char *magic_string() {return more_magic::MAGIC_NUMBER<ArithType>::name();}
    using value_type = ArithType;
    using pointer_type = ArithType *;
    static constexpr ArithType DEFAULT_VALUE = static_cast<ArithType>(DefaultValue);
    void set_default_value(ArithType val) {default_value_ = val;}
    DistanceMatrix(size_t n, ArithType default_value=DEFAULT_VALUE): data_((n * (n - 1)) >> 1), nelem_(n), default_value_(default_value) {
    } 
    pointer_type       data()       {return data_.data();}
    const ArithType   *data() const {return data_.data();}
    DistanceMatrix(DistanceMatrix &&other) = default;
    DistanceMatrix(const char *path, ArithType default_value=DEFAULT_VALUE): nelem_(0), default_value_(default_value) {
        this->read(path);
#if !NDEBUG
        for(const auto &el: data_) std::fprintf(stderr, "el at ind %zu: %f\n", &el - data_.data(), el);
#endif
    }
    DistanceMatrix(const DistanceMatrix &other):
            nelem_(other.nelem_) {
        data_.resize(other.data_.size());
        nelem_         = other.nelem_;
        default_value_ = other.default_value_;
        std::memcpy(data_.data(), other.data_.data(), data_.data.size() * sizeof(value_type));
    }
    size_t num_entries() const {return (nelem_ * (nelem_ - 1)) >> 1;}
#ifdef MANUAL_ARR
    // Manual calculation
    size_t index(size_t r, size_t c) const {
        assert(r < c);
        size_t ret = 0;
        for(size_t i(0); i < r; ++i) {
            ret += nelem_ - i - 1;
        }
        ret += c - r - 1;
        return ret;
    }
#endif
#define ARRAY_ACCESS(row, column) (((row) * (nelem_ * 2 - row - 1)) / 2 + column - (row + 1))
    value_type &operator()(size_t row, size_t column) {
        if(__builtin_expect(row == column, 0)) return default_value_;
#ifdef MANUAL_ARR
        const auto ind(row < column ? index(row, column): index(column, row)), acc(row < column ? ARRAY_ACCESS(row, column): ARRAY_ACCESS(column, row));
        assert(ind == acc || !std::fprintf(stderr, "ind: %zu. acc: %zu. r: %zu, c: %zu\n", ind, acc, row, column));
        if(row < column)
            return data_.at(ARRAY_ACCESS(row, column));
        return data_.at(ARRAY_ACCESS(column, row));
#else
        return data_[row < column ? ARRAY_ACCESS(row, column): ARRAY_ACCESS(column, row)];
#endif
    }
#undef ARRAY_ACCESS
    const value_type &operator()(size_t row, size_t column) const {
        return static_cast<const value_type &>(operator()(row, column));
    }
    value_type &operator[](size_t index) {
       return data_[index];
    }
    const value_type &operator[](size_t index) const {
       return data_[index];
    }
    void resize(size_t new_size) {
        if(new_size == nelem_) return; // Already done! Aren't we fast?
        if(new_size < nelem_) throw std::runtime_error("NotImplemented: shrinking.");
        data_.resize((new_size * (new_size - 1)) >> 1); // At least one number will be even, so we can just bitshift.
        nelem_ = new_size;
        std::fill(std::begin(data_), std::end(data_), static_cast<value_type>(-1)); // Invalid
    }
    void write(const std::string &path) const {
        return this->write(path.data());
    }
    void printf(std::FILE *fp, bool use_scientific=false) {
        std::array<std::array<char, 5>, 2> fmts;
        fmts[0] = {'%','l','f','\n','\0'};
        fmts[1] = {'%','l','f','\t','\0'};
        if(use_scientific) fmts[0][2] = fmts[1][2] = 'e';
        for(unsigned i(0); i < nelem_; ++i)
            for(unsigned j(0); j < nelem_; ++j)
                std::fprintf(fp, fmts[j != nelem_ - 1].data(), static_cast<double>(this->operator()(i, j)));
    }
    void write(const char *path) const {
        std::FILE *fp = std::fopen(std::strcmp(path, "-") ? path: "/dev/stdout", "wb");
        std::fputs(magic_string(), fp);
        std::fflush(fp);
        const int fn = fileno(fp);
        ::write(fn, &nelem_, sizeof(nelem_));
        ::write(fn, data_.data(), sizeof(ArithType) * data_.size());
        std::fclose(fp);
    }
    void read(const char *path) {
        std::FILE *fp = std::fopen(std::strcmp(path, "-") ? path: "/dev/stdin", "rb");
        char buf[128];
        std::string magic;
        if(std::fgets(buf, sizeof(buf), fp)) magic = buf;
        else throw std::runtime_error(std::string("Could not read magic string from file ") + path + ":((((((");
        magic.pop_back();
#if !NDEBUG
        std::fprintf(stderr, "Magic: %s\n", magic.data());
#endif
        if(magic != magic_string()) throw std::runtime_error(std::string("Read wrong magic string from file ") + path + ". Magic string: '" + magic + "', expected '" + magic_string() + "'");
        std::fread(&nelem_, sizeof(nelem_), 1, fp);
#if !NDEBUG
        std::fprintf(stderr, "Number of elements: %zu\n", nelem_);
        std::fprintf(stderr, "Number of entries: %zu\n", num_entries());
#endif
        data_.resize(num_entries());
        std::fread(data_.data(), sizeof(ArithType), data_.size(), fp);
        std::fclose(fp);
    }
    size_t size() const {return nelem_;}
};

} // namespace dm
