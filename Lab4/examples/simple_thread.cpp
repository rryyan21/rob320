#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>

void task(int i, std::string msg) {
    std::cout << "Task " << i << ": " << msg << std::endl;
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; i++) {
        threads.emplace_back(task, i, "Hello, world!");
    }
    for (std::thread& t : threads) {
        t.join();
    }
    return 0;
}