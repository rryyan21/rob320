#include <unistd.h>
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <semaphore>

std::queue<int> buffer;
std::counting_semaphore<10> sem(0);
std::mutex mtx;

void producer(int id) {
    for (int i = 0; i < 10; ++i) {
        mtx.lock();
        buffer.push(i);
        std::cout << "Producer " << id << " produced " << i << std::endl;
        mtx.unlock();
        sem.release();
        usleep(1000);
    }
}

void consumer(int id) {
    int item;
    for (int i = 0; i < 10; ++i) {
        sem.acquire();
        mtx.lock();
        item = buffer.front();
        buffer.pop();
        std::cout << "Consumer " << id << " consumed " << item << std::endl;
        mtx.unlock();
        usleep(2000);
    }
}

int main() {
    std::thread p1(producer, 1);
    std::thread p2(producer, 2);
    std::thread c1(consumer, 1);
    std::thread c2(consumer, 2);
    p1.join();
    p2.join();
    c1.join();
    c2.join();
    return 0;
}