#pragma once
#include <type_traits>
#include <cstring>
#include <cassert>
#include <memory>
#include <cstdlib>
#include <stdexcept>

namespace dm {

struct free_deleter{
    template <typename T>
    void operator()(T *p) const {
        std::free(const_cast<std::remove_const_t<T>*>(p));
    }
};

template<typename ArithType=float,
         size_t DefaultValue=0,
         typename=std::enable_if_t<std::is_arithmetic_v<ArithType>>
         >
class DistanceMatrix {
    ArithType  *data_;
    size_t     nelem_;
    ArithType   default_value_;
public:
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
};

} // namespace dm
