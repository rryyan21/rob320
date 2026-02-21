CXX = g++
CXXFLAGS = -std=c++17
PROGRAMS = simple_thread safe_thread unsafe_thread semaphore condition_variable safe_atomic unsafe_atomic

all: $(PROGRAMS)

simple_thread: simple_thread.cpp
	$(CXX) $(CXXFLAGS) -o simple_thread simple_thread.cpp -lpthread

safe_thread: safe_thread.cpp
	$(CXX) $(CXXFLAGS) -o safe_thread safe_thread.cpp -lpthread

unsafe_thread: unsafe_thread.cpp
	$(CXX) $(CXXFLAGS) -o unsafe_thread unsafe_thread.cpp -lpthread

semaphore: semaphore.cpp
	$(CXX) $(CXXFLAGS) -o semaphore semaphore.cpp -lpthread

condition_variable: condition_variable.cpp
	$(CXX) $(CXXFLAGS) -o condition_variable condition_variable.cpp -lpthread

safe_atomic: safe_atomic.cpp
	$(CXX) $(CXXFLAGS) -o safe_atomic safe_atomic.cpp -lpthread

unsafe_atomic: unsafe_atomic.cpp
	$(CXX) $(CXXFLAGS) -o unsafe_atomic unsafe_atomic.cpp -lpthread

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