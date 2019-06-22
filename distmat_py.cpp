#include "pybind11/pybind11.h"
#include "distmat.h"

namespace py = pybind11;

//template<typename...Args>
//using dm::DistanceMatrix<Args...>;
using dm::DistanceMatrix;
using FloatRef = float &;
using ConstFloatRef = float &;
PYBIND11_MODULE(distmat, m) {
    m.doc() = "distmat: hold a distance matrix in N choose 2 space with fast random access"; // optional module docstring
    py::class_<DistanceMatrix<float>> (m, "dm")
        .def(py::init<size_t>())
        .def(py::init<const char *>())
        .def("write", [](const DistanceMatrix<float> &x, const char *s) {x.write(s);})
        .def("read", [](DistanceMatrix<float> &x, const char *s) {x.read(s);})
        .def("get", [](DistanceMatrix<float> &x, size_t i, size_t j) -> FloatRef {return x(i, j);})
        .def("set", [](DistanceMatrix<float> &x, size_t i, size_t j, float val) {x(i, j) = val;})
        .def("printf", [](const DistanceMatrix<float> &x) {x.printf(stdout);})
        .def("printerr", [](const DistanceMatrix<float> &x) {x.printf(stderr);})
        .def("__str__", [](const DistanceMatrix<float> &x) {return x.to_string();});
#if 0
    m.def("jaccard_index", [](hll_t &h1, hll_t &h2) {
            return jaccard_index(h1, h2);
        }
    );
#endif
}
