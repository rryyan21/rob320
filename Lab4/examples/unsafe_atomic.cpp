#include <unistd.h>
#include <thread>
#include <vector>
#include <iostream>

int counter(0);

void increment(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        ++counter;
        usleep(100);
    }
}

int main() {
    const int num_threads = 10;
    const int iterations = 1000;

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(increment, iterations);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Counter: " << counter << std::endl;

    return 0;
}