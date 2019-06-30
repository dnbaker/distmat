#pragma once
#include <type_traits>
#include <cstring>
#include <vector>
#include <cassert>
#include <array>
#include <cstdlib>
#include <stdexcept>
#include <cstdint>
#include <limits>
#include <cinttypes>
#if ZWRAP_USE_ZSTD
#  include "zstd_zlibwrapper.h"
#else
#  include <zlib.h>
#endif
#include "unistd.h"

#ifndef INLINE
#  if defined(__GNUC__) || defined(__clang__)
#    define INLINE __attribute__((always_inline)) inline
#  elif defined(__CUDACC__)
#    define INLINE __forceinline__ inline
#  else
#    define INLINE inline
#  endif
#endif

#define STRINGIZER(x)   # x
#define TO_STRING(x)    STRINGIZER(x)

namespace std {
#if __cplusplus < 201703L
template <class C>
constexpr auto size(const C& c) -> decltype(c.size())
{
    return c.size();
}

template <class T, std::size_t N>
constexpr std::size_t size(const T (&array)[N]) noexcept
{
    return N;
}
#endif

static std::string to_string(__uint128_t num)
{
    std::string str;
    do {
        int digit = num % 10;
        num /= 10;
        str = std::to_string(digit) + str;
    } while(num);
    return str;
}
static std::string to_string(__int128_t n) {
    return to_string(__uint128_t(n)); // This is wrong, but just a placeholder.
}

template<> struct numeric_limits<__uint128_t> {
    static constexpr __uint128_t max() {return __uint128_t(-1);}
    static constexpr __uint128_t min() {return __uint128_t(0);}
};
template<> struct numeric_limits<__int128_t> {
    static constexpr __int128_t max() {return  (__int128) (((unsigned __int128) 1 << ((__SIZEOF_INT128__ * __CHAR_BIT__) - 1)) - 1);}
    static constexpr __int128_t min() {return (-max() - 1);}
};

} // namespace std
#undef TO_STRING
#undef STRINGIZER

namespace dm {

namespace more_magic {
template<typename ArithType>
struct MAGIC_NUMBER {
    const char *name() {
        throw std::runtime_error("NotImplemented");
        return "NOTIMPLEMENTED";
    }
};
enum MagicNumber: uint8_t {
    FLOAT,
    DOUBLE,
    UINT8_T,
    UINT16_T,
    UINT32_T,
    UINT64_T,
    UINT128_T,
    INT8_T,
    INT16_T,
    INT32_T,
    INT64_T,
    INT128_T
};
static constexpr const char *arr[] {
    "float",
    "double",
    "uint8_t",
    "uint16_t",
    "uint32_t",
    "uint64_t",
    "uint128_t",
    "int8_t",
    "int16_t",
    "int32_t",
    "int64_t",
    "int128_t",
};

#define DEC_MAGIC(type, STR, num) \
    template<> struct MAGIC_NUMBER<type> {\
        static constexpr MagicNumber magic_number = num;\
        static constexpr const char *name() {return "DM::" STR;}\
        static const std::string magic_name() {return std::string("DM::") + arr[magic_number];}\
    }

DEC_MAGIC(float, "float", FLOAT);
DEC_MAGIC(double,"double", DOUBLE);
DEC_MAGIC(uint8_t,"uint8_t", UINT8_T);
DEC_MAGIC(uint16_t,"uint16_t", UINT16_T);
DEC_MAGIC(uint32_t,"uint32_t", UINT32_T);
DEC_MAGIC(uint64_t,"uint64_t", UINT64_T);
DEC_MAGIC(__uint128_t,"uint128_t", UINT128_T);
DEC_MAGIC(int8_t,"int8_t", INT8_T);
DEC_MAGIC(int16_t,"int16_t", INT16_T);
DEC_MAGIC(int32_t,"int32_t", INT32_T);
DEC_MAGIC(int64_t,"int64_t", INT64_T);
DEC_MAGIC(__int128_t,"int128_t", INT128_T);

} // namespace more_magic

#undef DEC_MAGIC

/* *
 * DistanceMatrix holds an upper-triangular matrix.
 * You can access rows with row_span()
 * or individual entries with (i, j) notation (like Eigen/Blaze/&c.)
 * You can set the default value with set_default_value.
 *
*/
template<typename ArithType=float,
         size_t DefaultValue=0,
         bool force=false,
         typename=typename std::enable_if<std::is_arithmetic<ArithType>::value || std::is_same<ArithType,__uint128_t>::value || std::is_same<ArithType,__int128_t>::value || force>::type
         >
class DistanceMatrix {
    std::vector<ArithType> data_;
    uint64_t  nelem_;
    ArithType default_value_;
public:
    static constexpr const char *magic_string() {return more_magic::MAGIC_NUMBER<ArithType>::name();}
    static constexpr more_magic::MagicNumber magic_number() {return more_magic::MAGIC_NUMBER<ArithType>::magic_number;}
    using value_type = ArithType;
    using pointer_type = ArithType *;
    using const_pointer_type = const ArithType *;
    static constexpr ArithType DEFAULT_VALUE = static_cast<ArithType>(DefaultValue);
    void set_default_value(ArithType val) {default_value_ = val;}
    DistanceMatrix(size_t n, ArithType default_value=DEFAULT_VALUE): data_((n * (n - 1)) >> 1), nelem_(n), default_value_(default_value) {
    }
    DistanceMatrix(): DistanceMatrix(size_t(0), DEFAULT_VALUE) {}
    pointer_type       data()       {return data_.data();}
    const_pointer_type data() const {return data_.data();}
    DistanceMatrix(DistanceMatrix &&other) = default;
    DistanceMatrix(const char *path, ArithType default_value=DEFAULT_VALUE): nelem_(0), default_value_(default_value) {
        this->read(path);
#if !NDEBUG
        //for(const auto &el: data_) std::fprintf(stderr, "el at ind %zu: %f\n", &el - data_.data(), el);
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
    INLINE value_type &operator()(size_t row, size_t column) {
        if(__builtin_expect(row == column, 0)) return default_value_;
        return data_[row < column ? ARRAY_ACCESS(row, column): ARRAY_ACCESS(column, row)];
    }
    INLINE const value_type &operator()(size_t row, size_t column) const {
        if(__builtin_expect(row == column, 0)) return default_value_;
        return data_[row < column ? ARRAY_ACCESS(row, column): ARRAY_ACCESS(column, row)];
    }
    pointer_type row_ptr(size_t row) {
        auto ret = data_.data() + nelem_ * row - (row * (row + 1) / 2);
        return ret;
    }
    const_pointer_type row_ptr(size_t row) const {
        auto ret = data_.data() + nelem_ * row - (row * (row + 1) / 2);
        return ret;
    }
    std::pair<pointer_type, size_t> row_span(size_t i) {
        return std::make_pair(row_ptr(i), nelem_ - i - 1);
    }
    std::pair<const_pointer_type, size_t> row_span(size_t i) const {
        return std::make_pair(row_ptr(i), nelem_ - i - 1);
    }
#undef ARRAY_ACCESS
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
    std::string to_string(bool use_scientific=false, const std::vector<std::string> *labels=nullptr) const {
        std::string ret;
        ret.reserve(size() * size() * 6 + (labels ? size_t(10 * labels->size()): size_t(0)));
        if(labels) {
            ret = "##Labels";
            for(const auto &l: *labels)
                ret += '\t', ret += l;
            ret += '\n';
        }
        for(size_t i = 0; i < size(); ++i) {
            if(labels)
                ret += labels->operator[](i), ret += '\t';
            for(size_t j = 0; j < size(); ++j) {
                ret += std::to_string(this->operator()(i, j)), ret += '\t';
            }
            ret.back() = '\n'; // Extra tab is now a newline
        }
        return ret;
    }
    void printf(gzFile fp, bool use_scientific=false, const std::vector<std::string> *labels=nullptr) const {
        if(labels) {
            gzprintf(fp, "#Names");
            for(const auto &s: *labels) {
                gzputc(fp, '\t');
                gzwrite(fp, s.data(), s.size());
            }
            gzputc(fp, '\n');
        }
        std::array<std::array<char, 5>, 2> fmts{{'%','l','f','\n','\0'}, {'%','l','f','\t','\0'}};
        if(use_scientific) fmts[0][2] = fmts[1][2] = 'e';
        for(unsigned i(0); i < nelem_; ++i) {
            if(labels) {
                const auto &label = (*labels)[i];
                gzwrite(fp, label.data(), label.size());
                gzputc(fp, '\t');
            }
            for(unsigned j(0); j < nelem_; ++j) {
                gzprintf(fp, fmts[j != nelem_ - 1].data(), static_cast<double>(this->operator()(i, j)));
            }
        }
    }
    void printf(std::FILE *fp, bool use_scientific=false, const std::vector<std::string> *labels=nullptr) const {
        if(labels) {
            fprintf(fp, "#Names");
            for(const auto &s: *labels) {
                fputc('\t', fp);
                fwrite(s.data(), s.size(), 1, fp);
            }
            fputc('\n', fp);
        }
        std::array<std::array<char, 5>, 2> fmts;
        fmts[0] = {'%','l','f','\n','\0'};
        fmts[1] = {'%','l','f','\t','\0'};
        if(use_scientific) fmts[0][2] = fmts[1][2] = 'e';
        for(unsigned i(0); i < nelem_; ++i) {
            if(labels) {
                const auto &label = labels->operator[](i);
                fwrite(label.data(), label.size(), 1, fp);
                fputc('\t', fp);
            }
            for(unsigned j(0); j < nelem_; ++j) {
                fprintf(fp, fmts[j != nelem_ - 1].data(), static_cast<double>(this->operator()(i, j)));
            }
        }
    }
    size_t write(const char *path, int compression_level=0) const {
        std::string fmt = compression_level ? std::string("wT"): (std::string("wb") + std::to_string(compression_level % 10));
        gzFile fp = gzopen(std::strcmp(path, "-") ? path: "/dev/stdout", fmt.data());
        if(!fp) throw std::runtime_error(std::string("Could not open file at ") + path);
        size_t ret = write(fp);
        gzclose(fp);
        return ret;
    }
    size_t write(gzFile fp) const {
        size_t ret = gzputc(fp, magic_number());
        ret += gzwrite(fp, &nelem_, sizeof(nelem_));
        ret += gzwrite(fp, data_.data(), sizeof(ArithType) * data_.size());
        return ret;
    }
    size_t write(std::FILE *fp) const {
        size_t ret = std::fputc(magic_number(), fp);
        int fn = fileno(fp);
        ret += ::write(fn, &nelem_, sizeof(nelem_));
        ret += ::write(fn, data_.data(), sizeof(ArithType) * data_.size());
        return ret;
    }
    void read(const char *path) {
        path = std::strcmp(path, "-") ? path: "/dev/stdin";
        gzFile fp = gzopen(path, "rb");
        if(fp == nullptr) throw std::runtime_error(std::string("Could not open file at ") + path);
        more_magic::MagicNumber magic = more_magic::MagicNumber(gzgetc(fp));
        if(magic != magic_number()) {
            char buf[256];
            std::sprintf(buf, "Wrong magic number read from file (%d/%s), expected (%d/%s)\n", magic, more_magic::arr[magic], magic_number(), magic_string());
            throw std::runtime_error(buf);
        }
        gzread(fp, &nelem_, sizeof(nelem_));
#if !NDEBUG
        std::fprintf(stderr, "Number of elements: %zu\n", nelem_);
        std::fprintf(stderr, "Number of entries: %zu\n", num_entries());
#endif
        data_.resize(num_entries());
        gzread(fp, data_.data(), sizeof(ArithType) * data_.size());
        gzclose(fp);
    }
    size_t size() const {return nelem_;}
    bool operator==(const DistanceMatrix &o) const {
        return nelem_ == o.nelem_ &&
            (std::memcmp(data_.data(), o.data_.data(), data_.size() * sizeof(ArithType)) == 0);
    }
};

} // namespace dm
