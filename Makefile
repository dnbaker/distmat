CXX?=g++

PYTHON?=$(shell which python)
PYCONFIG?=$(PYTHON)-config
EXT=$(shell $(PYCONFIG) --extension-suffix)
INCLUDE+=
PYINCLUDE=$(shell $(PYCONFIG) --includes) -I. -Ipybind11/include $(INCLUDE)
LIB=-lz
PYLIB=-L$(shell $(PYCONFIG) --prefix)/lib $(shell $(PYCONFIG) --libs)  #$(shell $(PYCONFIG) --ldflags)
ifeq ($(shell uname),Darwin)
    UDSTR=-undefined dynamic_lookup
else
    UDSTR=
endif
FLAGS=$(INCLUDE) -std=c++14 -O3 -march=native $(LIB) $(UDSTR)

PREFIX?=/usr/local


all: printmat serialization span
%: src/%.cpp distmat.h
	$(CXX) $(FLAGS) $< -o $@ -I. -lz

%: test/%.cpp distmat.h
	$(CXX) $(FLAGS) $< -o $@ -I. -lz  -std=c++14

python: distmat_py.cpp
	echo "TODO: rewrite with setup.py" && \
	$(CXX) $(FLAGS) -shared -fPIC $(PYINCLUDE) $< -o distmat$(EXT) $(LIB) $(PYLIB) && touch python 

install: distmat.h
	install -d $(DESTDIR)$(PREFIX)/include/distmat && \
    install -m 644 distmat.h $(DESTDIR)$(PREFIX)/include/distmat && \
    cd pybind11 && mkdir -p build && cd build && cmake .. && make && make install

clean:
	rm -f distmat$(EXT) printmat serialization span
