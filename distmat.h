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

template<typename ArithType>
struct MAGIC_NUMBER {
    const char *name() {
        throw std::runtime_error("NotImplemented");
        return "NOTIMPLEMENTED";
    }
};

#define DEC_MAGIC(type, STR) \
    template<>\
    struct MAGIC_NUMBER<type> {\
        const char *name() const {\
            return "DM::" STR;\
        }\
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

#undef DEC_MAGIC
template<typename ArithType=float,
         size_t DefaultValue=0,
         typename=std::enable_if_t<std::is_arithmetic_v<ArithType>>
         >
class DistanceMatrix {
    std::vector<ArithType> data_;
    size_t     nelem_;
    ArithType   default_value_;
    static const MAGIC_NUMBER<ArithType> magic_;
public:
    const char *magic_string() const {
        return magic_.name();
    }
    using value_type = ArithType;
    using pointer_type = ArithType *;
    static constexpr ArithType DEFAULT_VALUE = static_cast<ArithType>(DefaultValue);
    void set_default_value(ArithType val) {default_value_ = val;}
    DistanceMatrix(size_t n, ArithType default_value=DEFAULT_VALUE): data_((n * (n - 1)) >> 1), nelem_(n), default_value_(default_value) {
    } 
    pointer_type       data()       {return data_.data();}
    const ArithType   *data() const {return data_.data();}
    DistanceMatrix(DistanceMatrix &&other) {
        std::memcpy(&other, this, sizeof(*this));
        std::memset(&other, 0, sizeof(other));
    }
    DistanceMatrix(const char *path, ArithType default_value=DEFAULT_VALUE): data_(nullptr), nelem_(0), default_value_(default_value) {
        this->read(path);
    }
    DistanceMatrix(const DistanceMatrix &other):
            data_(static_cast<pointer_type>(std::malloc(other.num_entries()))), nelem_(other.nelem_) {
        if(data_ == nullptr) throw std::bad_alloc();
        nelem_         = other.nelem_;
        default_value_ = other.default_value_;
        std::memcpy(data_.data(), other.data_.data(), num_entries() * sizeof(value_type));
    }
    size_t num_entries() const {return (nelem_ * (nelem_ - 1)) >> 1;}
#define ARRAY_ACCESS(row, column) (((row) * (nelem_ * 2 - row - 1)) / 2 + column - (row + 1))
    value_type &operator()(size_t row, size_t column) {
        if(__builtin_expect(row == column, 0)) return default_value_;
#if !NDEBUG
        if(row < column)
            return data_.at(ARRAY_ACCESS(row, column));
        return data_.at(ARRAY_ACCESS(column, row));
#else
        if(row < column) return data_[ARRAY_ACCESS(row, column)];
        else             return data_[ARRAY_ACCESS(column, row)];
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
        if(new_size < nelem_) throw std::runtime_error("NotImplemented: shrinking.");
        if(new_size == nelem_) return;
        auto tmp = std::realloc(data_, sizeof(value_type) * (new_size * (new_size - 1)));
        if(tmp == nullptr) throw std::bad_alloc();
        nelem_ = new_size;
        data_ = tmp;
        std::fill(std::begin(data_), std::end(data_), static_cast<value_type>(-1));
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
                std::fprintf(fp, fmts[j != nelem_ - 1].data(), this->operator()(i, j));
    }
    void write(const char *path) const {
        std::FILE *fp = std::fopen(path, "wb");
        std::fputs(magic_string(), fp);
        const int fn = fileno(fp);
        ::write(fn, &nelem_, sizeof(nelem_));
        ::write(fn, &default_value_, sizeof(default_value_));
        ::write(fn, data_, sizeof(ArithType) * nelem_);
        std::fclose(fp);
    }
    void read(const char *path) {
        std::FILE *fp = std::fopen(path, "rb");
        char buf[128];
        std::string magic;
        if(std::fgets(buf, sizeof(buf), fp)) magic = buf;
        else throw std::runtime_error(std::string("Could not read magic string from file ") + path + ":((((((");
        if(magic != magic_string()) throw std::runtime_error(std::string("Read wrong magic string from file ") + path + ":((((((: " + magic + ", expected " + magic_string());
        const int fn = fileno(fp);
        ::read(fn, &nelem_, sizeof(nelem_));
        if((data_ = static_cast<ArithType *>(std::realloc(data_, (nelem_ * nelem_ - 1)>>1))) == nullptr) throw std::bad_alloc();
        ::read(fn, static_cast<void *>(&default_value_), sizeof(default_value_));
        ::read(fn, data_, sizeof(ArithType) * nelem_);
        std::fclose(fp);
    }
    size_t size() const {return nelem_;}
    // TODO: Add serialization.
};

} // namespace dm
