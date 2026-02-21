CXX = g++
CXXFLAGS = -std=c++17
DEBUGFLAGS = -g
PROGRAMS = matmul matmul_O3 path_planning path_planning_O3

all: $(PROGRAMS)

matmul: matmul.cpp
	$(CXX) $(CXXFLAGS) -o matmul matmul.cpp

matmul_O3: matmul.cpp
	$(CXX) $(CXXFLAGS) -o matmul_O3 matmul.cpp -O3

path_planning: path_planning.cpp path_planning.hpp
	$(CXX) $(CXXFLAGS) -o path_planning path_planning.cpp

path_planning_O3: path_planning.cpp path_planning.hpp
	$(CXX) $(CXXFLAGS) -o path_planning_O3 path_planning.cpp -O3

clean:
	rm -f $(PROGRAMS)

.PHONY: all clean