#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <iostream>
#include <condition_variable>
#include <functional>

class ThreadPool
{
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    // This templated function enqueues a task into the thread pool.
    // It uses variadic templates to accept any number of arguments.
    // The arguments are bound to the function using std::bind.
    template <typename Func, typename... Args>
    void enqueue(Func func, Args... args);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;

    void worker_thread();
};

ThreadPool::ThreadPool(size_t num_threads) : stop(false)
{
    // Create num_threads worker threads and place them in the workers vector.
    for (size_t i = 0; i < num_threads; ++i)
    {
        workers.emplace_back(&ThreadPool::worker_thread, this);
    }
}

ThreadPool::~ThreadPool()
{
    // TODO: Lock the queue_mutex, set the stop flag to true, and unlock the
    //       mutex.

    queue_mutex.lock();
    stop = true;
    queue_mutex.unlock();

    // TODO: Notify all worker threads to wake up.
    // Hint: Use the condition variable to notify all threads.
    condition.notify_all();

    // TODO: Join all worker threads.
    for (std::thread &worker : workers)
    {
        worker.join();
    }
}

template <typename Func, typename... Args>
void ThreadPool::enqueue(Func func, Args... args)
{
    // TODO: Lock the queue_mutex
    queue_mutex.lock();

    // The tasks vector expects a std::function<void()> object. This is
    // a function with no arguments. Our function may have arguments, so
    // we must bind the function and arguments together in order to "cast"
    // it to a std::function<void()> object.
    // For more info on std::bind, see: https://en.cppreference.com/w/cpp/utility/functional/bind
    tasks.emplace(std::bind(func, args...));

    // TODO: Unlock the queue_mutex
    queue_mutex.unlock();

    // TODO: Notify one worker thread to wake up and process the task.
    // Hint: Use the condition variable to notify one thread.
    condition.notify_one();
}

void ThreadPool::worker_thread()
{
    while (true)
    {
        std::function<void()> task;
        {
            // TODO: Declare a std::unique_lock with the queue_mutex.
            std::unique_lock<std::mutex> lock(queue_mutex);
            // This loop is here to handle spurious wake-ups. If the condition
            // variable is notified but the condition is not met, the thread
            // will go back to sleep. This is a standard pattern for using
            // condition variables.
            while (!stop && tasks.empty())
            {
                // TODO: Wait for a notification.
                condition.wait(lock);
            }

            // If the stop flag is set and the task queue is empty, return.
            if (stop && tasks.empty())
            {
                return;
            }

            // Get the next task from the queue. Use std::move to transfer
            // ownership (this is faster than copying)
            task = std::move(tasks.front());

            // Remove the task from the queue.
            tasks.pop();
        }

        // Execute the task.
        task();
    }
}