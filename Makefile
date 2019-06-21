CXX?=g++

PYCONFIG?=python3.5-config
EXT=$(shell $(PYCONFIG) --extension-suffix)
INCLUDE+=-I /usr/include/python3.5m/
FLAGS=$(INCLUDE) -std=c++14 -O3 -march=native


all: printmat serialization span
%: src/%.cpp distmat.h
	$(CXX) $(FLAGS) $< -o $@ -I. -lz

%: test/%.cpp distmat.h
	$(CXX) $(FLAGS) $< -o $@ -I. -lz  -std=c++14

python: distmat_py.cpp
	$(CXX) $(FLAGS) -shared -fPIC -I. -I pybind11/include $< -o distmat$(EXT) && touch python
