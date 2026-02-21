#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <fstream>

#include "thread_pool.hpp"

// Define a matrix type as a vector of vectors of floats
using Matrix = std::vector<std::vector<float>>;

// Print matrix elements to the console (for debugging)
void print_matrix(const Matrix &matrix)
{
    for (const auto &row : matrix)
    {
        for (const auto &elem : row)
        {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }
}

// Compare two matrices element-wise within a given error margin (epsilon)
bool compare_matrices(const Matrix &A, const Matrix &B, float epsilon = 1e-6)
{
    if (A.size() != B.size() || A[0].size() != B[0].size())
    {
        return false;
    }
    for (size_t i = 0; i < A.size(); i++)
    {
        for (size_t j = 0; j < A[0].size(); j++)
        {
            if (std::abs(A[i][j] - B[i][j]) > epsilon)
            {
                return false;
            }
        }
    }
    return true;
}

// Generate a random matrix with the given number of rows and columns
// The matrix elements are random values between -100.0 and 100.0
Matrix random_matrix(size_t rows, size_t cols)
{
    Matrix matrix(rows, std::vector<float>(cols, 0));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0, 1.0);

    for (size_t i = 0; i < rows; i++)
    {
        for (size_t j = 0; j < cols; j++)
        {
            matrix[i][j] = (int)(100 * dis(gen));
        }
    }
    return matrix;
}

// Multiply two matrices using the naive algorithm
Matrix multiply_naive(const Matrix &A, const Matrix &B)
{
    Matrix result(A.size(), std::vector<float>(B[0].size(), 0));
    for (size_t i = 0; i < A.size(); i++)
    {
        for (size_t j = 0; j < B[0].size(); j++)
        {
            for (size_t k = 0; k < B.size(); k++)
            {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return result;
}

// Speed up the naive matrix multiplication algorithm by using cache-friendly
// memory access patterns and minimizing memory loads and stores
Matrix multiply_fast(const Matrix &A, const Matrix &B)
{
    if (A[0].size() != B.size())
    {
        throw std::invalid_argument("Invalid matrix dimensions");
    }

    const size_t rows = A.size();
    const size_t cols = B[0].size();
    const size_t common = B.size();
    Matrix result(rows, std::vector<float>(cols, 0));
    for (size_t i = 0; i < rows; i++)
    {
        std::vector<float> &result_i = result[i];
        const std::vector<float> &A_i = A[i];
        for (size_t k = 0; k < common; k++)
        {
            float Aik = A_i[k];
            const std::vector<float> &B_k = B[k];
            for (size_t j = 0; j < cols; j++)
            {
                result_i[j] += Aik * B_k[j];
            }
        }
    }
    return result;
}

void matrix_multiply_task(const std::vector<float> &A_i,
                          const Matrix &B,
                          std::vector<float> &result_i,
                          size_t common,
                          size_t cols)
{
    // TODO: Multiply row i of matrix A with the columns of matrix B
    //       and store the result in row i of the result matrix
    //       Hint: Reuse the code from the multiply_fast function to
    //             write the loop that calculates the result matrix row

    for (size_t k = 0; k < common; k++)
    {
        float Aik = A_i[k];
        const std::vector<float> &B_k = B[k];
        for (size_t j = 0; j < cols; j++)
        {
            result_i[j] += Aik * B_k[j];
        }
    }
}

Matrix multiply_threads(const Matrix &A, const Matrix &B)
{
    if (A[0].size() != B.size())
    {
        throw std::invalid_argument("Invalid matrix dimensions");
    }

    const size_t rows = A.size();
    const size_t cols = B[0].size();
    const size_t common = B.size();
    Matrix result(rows, std::vector<float>(cols, 0));
    std::vector<std::thread> threads(rows);

    // TODO: Speed up the matrix multiplication algorithm by using a thread to
    //       calculate each row of the result matrix concurrently
    //       Hint: Use the matrix_multiply_task function as the thread
    //       Hint: Use the std::thread constructor to create threads
    //       Hint: Use std::ref to pass arguments by reference to the threads
    //       Hint: Use std::thread::join to wait for all threads to finish
    //       Hint: Reuse the code from the multiply_fast function to write the
    //             loop that creates the threads
    for (size_t i = 0; i < rows; i++)
    {
        threads[i] = std::thread(matrix_multiply_task, std::ref(A[i]), std::ref(B), std::ref(result[i]), common, cols);
    }
    // TODO: Join all threads
    for (std::thread &t : threads)
    {
        t.join();
    }
    return result;
}

Matrix multiply_thread_pool(const Matrix &A, const Matrix &B)
{
    if (A[0].size() != B.size())
    {
        throw std::invalid_argument("Invalid matrix dimensions");
    }

    // Get the number of hardware threads (usually the number of CPU cores)
    size_t num_threads = std::thread::hardware_concurrency();
    ThreadPool pool(num_threads);

    const size_t rows = A.size();
    const size_t cols = B[0].size();
    const size_t common = B.size();
    Matrix result(rows, std::vector<float>(cols, 0));

    // TODO: Speed up the matrix multiplication algorithm by using a thread pool
    //       to compute the result matrix.
    // Hint: Rather than using std::thread directly like in multiply_threads,
    //       use the ThreadPool class to enqueue tasks to be executed by the
    //       threads in the pool.
    for (size_t i = 0; i < rows; i++)
    {
        pool.enqueue(matrix_multiply_task, std::ref(A[i]), std::ref(B), std::ref(result[i]), common, cols);
    }
    // The destructor of the ThreadPool class will wait for all tasks to finish
    return result;
}

#pragma optimize("", off)
void test_multiply_speeds(size_t rows, size_t cols, size_t num_iters)
{
    static bool header_written = false;
    Matrix A = random_matrix(rows, cols);
    Matrix B = random_matrix(cols, rows);

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_iters; i++)
    {
        multiply_naive(A, B);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto naive_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << std::fixed << std::setprecision(0);

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_iters; i++)
    {
        multiply_fast(A, B);
    }
    end = std::chrono::high_resolution_clock::now();
    auto fast_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_iters; i++)
    {
        multiply_threads(A, B);
    }
    end = std::chrono::high_resolution_clock::now();
    auto threads_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_iters; i++)
    {
        multiply_thread_pool(A, B);
    }
    end = std::chrono::high_resolution_clock::now();
    auto thread_pool_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    std::cout << std::setprecision(2);
    uint64_t naive_ns = naive_duration.count() / num_iters;
    uint64_t fast_ns = fast_duration.count() / num_iters;
    uint64_t threads_ns = threads_duration.count() / num_iters;
    uint64_t thread_pool_ns = thread_pool_duration.count() / num_iters;

    float naive_to_fast = (float)naive_duration.count() / fast_duration.count();
    float fast_to_threads = (float)fast_duration.count() / threads_duration.count();
    float naive_to_threads = (float)naive_duration.count() / threads_duration.count();
    float fast_to_thread_pool = (float)fast_duration.count() / thread_pool_duration.count();
    float naive_to_thread_pool = (float)naive_duration.count() / thread_pool_duration.count();

    if (!header_written)
    {
        std::cout << "rows,cols,naive_ns,fast_ns,fast_ns,naive_to_fast,fast_to_threads,naive_to_threads,fast_to_thread_pool,naive_to_thread_pool" << std::endl;
        header_written = true;
    }

    std::cout << rows << "," << cols << "," << naive_ns << "," << fast_ns << "," << fast_ns << ","
              << naive_to_fast << "," << fast_to_threads << "," << naive_to_threads << ","
              << fast_to_thread_pool << "," << naive_to_thread_pool << std::endl;
}
#pragma optimize("", on)

int main()
{
    Matrix A = {{74, -91, 90, 11},
                {-80, 20, 12, -19},
                {-28, 56, 64, 93},
                {9, -20, -95, 73},
                {-35, -66, 74, -31},
                {-80, 9, -14, 31},
                {-37, 72, -45, 36},
                {0, 76, 96, 66}};

    Matrix B = {{62, -14, -16, 71, -28, -16, 89, -64},
                {-33, -68, -75, 93, 25, 29, 80, 59},
                {57, -15, -58, -8, -25, 82, -25, -37},
                {8, -61, -28, -3, 0, -26, -81, 3}};

    Matrix C = {{12809, 3131, 113, -3962, -6597, 3271, -3835, -13402},
                {-5088, 739, -384, -3859, 2440, 3338, -4281, 5799},
                {808, -10049, -10068, 2429, 584, 4902, -7145, 3007},
                {-3613, -1794, 4822, -680, 1623, -10412, -4337, 1978},
                {3978, 5759, 2086, -9122, -2520, 5520, -7734, -4485},
                {-5807, -1173, 549, -4824, 2815, -413, -8561, 6262},
                {-6947, -5899, -3206, 4321, 3961, -1946, 676, 8389},
                {3492, -10634, -13116, 6102, -500, 8360, -1666, 1130}};

    Matrix result_naive = multiply_naive(A, B);
    if (!compare_matrices(result_naive, C))
    {
        std::cout << "ERROR: Naive matrix multiplication is incorrect" << std::endl;
        return 1;
    }

    Matrix result = multiply_fast(A, B);
    if (!compare_matrices(result, C))
    {
        std::cout << "ERROR: Fast matrix multiplication is incorrect" << std::endl;
        return 1;
    }

    Matrix result_fast = multiply_threads(A, B);
    if (!compare_matrices(result_fast, C))
    {
        std::cout << "ERROR: Threads matrix multiplication is incorrect" << std::endl;
        return 1;
    }

    Matrix result_thread_pool = multiply_thread_pool(A, B);
    if (!compare_matrices(result_thread_pool, C))
    {
        std::cout << "ERROR: Thread pool matrix multiplication is incorrect" << std::endl;
        return 1;
    }

    for (int i = 4; i <= 512; i *= 2)
    {
        test_multiply_speeds(i, i, 50);
    }

    return 0;
}