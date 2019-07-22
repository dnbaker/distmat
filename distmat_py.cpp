#include "pybind11/pybind11.h"
#include <string>
#include <stdexcept>
#include "distmat.h"

namespace py = pybind11;
using dm::DistanceMatrix;
PYBIND11_MODULE(distmat, m) {
    m.doc() = "distmat: hold a distance matrix in N choose 2 space with fast random access"; // optional module docstring
#define DEC_TYPE(TYPE, suffix) \
    py::class_<DistanceMatrix<TYPE>> (m, "dm" suffix, "dm " suffix " (x): if x is a str, load binary matrix from file. If x is an integer, create an empty triangular distance matrix.")\
        .def(py::init<size_t>())\
        .def(py::init<const char *>())\
        .def("write", [](const DistanceMatrix<TYPE> &x, const char *s) {x.write(s);})\
        .def("read", [](DistanceMatrix<TYPE> &x, const char *s) {x.read(s);})\
        .def("get", [](DistanceMatrix<TYPE> &x, size_t i, size_t j) -> TYPE &{return x(i, j);})\
        .def("set", [](DistanceMatrix<TYPE> &x, size_t i, size_t j, TYPE val) {x(i, j) = val;})\
        .def("printf", [](const DistanceMatrix<TYPE> &x) {x.printf(stdout);})\
        .def("printerr", [](const DistanceMatrix<TYPE> &x) {x.printf(stderr);})\
        .def("__str__", [](const DistanceMatrix<TYPE> &x) {return x.to_string();})
    DEC_TYPE(float, "_float");
    DEC_TYPE(double, "_double");
    DEC_TYPE(int8_t, "_int8_t");
    DEC_TYPE(int16_t, "_int16_t");
    DEC_TYPE(int32_t, "_int32_t");
    DEC_TYPE(int64_t, "_int64_t");
    DEC_TYPE(uint8_t, "_uint8_t");
    DEC_TYPE(uint16_t, "_uint16_t");
    DEC_TYPE(uint32_t, "_uint32_t");
    DEC_TYPE(uint64_t, "_uint64_t");
    DEC_TYPE(__uint128_t, "_uint128_t")
        .def("set_halves", [](DistanceMatrix<float> &x, size_t i, size_t j, uint64_t v1, uint64_t v2) {x(i, j) = (__uint128_t(v1) << 64) | v2;});
    DEC_TYPE(__int128_t, "_int128_t")
        .def("set_halves", [](DistanceMatrix<float> &x, size_t i, size_t j, int64_t v1, int64_t v2) {x(i, j) = (__int128_t(v1) << 64) | v2;});
}
