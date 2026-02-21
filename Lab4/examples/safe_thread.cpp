#include <unistd.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;
std::vector<int> vec;

void safe_thread(int start, int end) {
    for (int i = start; i < end; i++) {
        mtx.lock();
        vec.push_back(i);
        mtx.unlock();
        usleep(10);
    }
}

int main() {
    std::thread a(safe_thread, 0, 1000);
    std::thread b(safe_thread, 1000, 2000);
    a.join();
    b.join();

    for (int i : vec) {
        std::cout << i << " ";
    }
}