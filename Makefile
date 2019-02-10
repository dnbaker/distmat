CXX?=g++
%: src/%.cpp distmat.h
	$(CXX) -march=native -O3 $< -o $@ -I. -lz 

%: test/%.cpp distmat.h
	$(CXX) -march=native -O3 $< -o $@ -I. -lz 
