#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>

std::vector<int> vec;

void unsafe_thread(int start, int end) {
    for (int i = start; i < end; i++) {
        vec.push_back(i);
    }
}

int main() {
    std::thread a(unsafe_thread, 0, 1000);
    std::thread b(unsafe_thread, 1000, 2000);
    a.join();
    b.join();

    for (int i : vec) {
        std::cout << i << " ";
    }
}