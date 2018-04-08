#pragma once
#include <type_traits>
#include <cstring>
#include <cassert>
#include <memory>
#include <cstdlib>
#include <stdexcept>
#include <cstdint>
#include "unistd.h"

namespace dm {

template<typename ArithType>
class MAGIC_NUMBER {
    const char *name() {
        throw std::runtime_error("NotImplemented");
        return "NOTIMPLEMENTED";
    }
};

#define DEC_MAGIC(type, STR) \
    template<>\
    class MAGIC_NUMBER<type> {\
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
    ArithType  *data_;
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
    DistanceMatrix(size_t n): data_(static_cast<pointer_type>(std::malloc(n * n - 1))), nelem_(n), default_value_(DefaultValue) {
        if(data_ == nullptr) throw std::bad_alloc();
    } 
    pointer_type       data()       {return data_;}
    const pointer_type data() const {return data_;}
    DistanceMatrix(DistanceMatrix &&other) {
        std::memcpy(&other, this);
        std::memset(&other, 0, sizeof(other));
    }
    DistanceMatrix(const DistanceMatrix &other):
            data_(static_cast<pointer_type>(std::malloc(other.num_entries()))), nelem_(other.nelem_) {
        if(data_ == nullptr) throw std::bad_alloc();
        nelem_         = other.nelem_;
        default_value_ = other.default_value_;
        std::memcpy(data_, other.data_, num_entries() * sizeof(value_type));
    }
    ~DistanceMatrix() {std::free(data_);}
    size_t num_entries() const {return nelem_ * (nelem_ - 1);}
#define ARRAY_ACCESS(row, column) (((nelem_ - 1) * row - ((row * (row - 1)) >> 1)) /* Start of row */ + column - 1)
    value_type &operator()(size_t row, size_t column) {
        if(__builtin_expect(row == column, 0)) return default_value_;
        if(row < column) return data_[ARRAY_ACCESS(row, column)];
        else             return data_[ARRAY_ACCESS(column, row)];
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
        std::fill(data_, data_ + num_entries(), static_cast<value_type>(-1));
    }
    void write(const std::string &path) const {
        return this->write(path.data());
    }
    void printf(std::FILE *fp) {
        for(unsigned i(0); i < nelem_; ++i)
            for(unsigned j(0); j < nelem_; ++j)
                std::fprintf(fp, j == nelem_ - 1 ? "%lf\n": "%lf\t", this->operator(i, j));
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
    void read(const char *path) const {
        std::FILE *fp = std::fopen(path, "rb");
        char buf[128];
        std::string magic;
        if(std::fgets(buf, sizeof(buf), fp)) magic = buf;
        else throw std::runtime_error(std::string("Could not read magic string from file ") + path + ":((((((");
        if(magic != magic_string()) throw std::runtime_error(std::string("Read wrong magic string from file ") + path + ":((((((: " + magic + ", expected " + magic_string());
        const int fn = fileno(fp);
        ::read(fn, &nelem_, sizeof(nelem_));
        ::read(fp, &default_value_, sizeof(default_value_));
        ::read(fn, data_, sizeof(ArithType) * nelem_);
        std::fclose(fp);
    }
    // TODO: Add serialization.
};

} // namespace dm
