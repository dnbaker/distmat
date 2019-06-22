CXX?=g++

PYCONFIG?=python3.7-config
EXT=$(shell $(PYCONFIG) --extension-suffix)
INCLUDE+=
PYINCLUDE=$(shell $(PYCONFIG) --includes) -I. -Ipybind11/include
LIB=-lz
PYLIB=-L$(shell $(PYCONFIG) --prefix)/lib -lpython3.7 #$(shell $(PYCONFIG) --ldflags)
FLAGS=$(INCLUDE) -std=c++14 -O3 -march=native $(LIB)


all: printmat serialization span
%: src/%.cpp distmat.h
	$(CXX) $(FLAGS) $< -o $@ -I. -lz

%: test/%.cpp distmat.h
	$(CXX) $(FLAGS) $< -o $@ -I. -lz  -std=c++14

python: distmat_py.cpp
	$(CXX) $(FLAGS) -shared -fPIC $(PYINCLUDE) $< -o distmat$(EXT) $(LIB) $(PYLIB) && touch python 

clean:
	rm -f distmat$(EXT) printmat serialization span
