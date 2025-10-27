#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
#include <string.h>
#include <thread>

#include "sums.h"

void get_chunk_indices(int64_t N, int num_threads, int thread_id, int64_t& start, int64_t& end) {
    int64_t chunk_size = (N + num_threads - 1) / num_threads;
    start = thread_id * chunk_size;
    end = std::min(start + chunk_size, N);
}

void 
setup(int64_t N, int64_t A[])
{
    // printf(" inside sum_vector problem_setup, N=%lld \n", N);
    
    // Perlmutter node has 128 logical threads, but we can ask just in case
    const int num_threads = std::thread::hardware_concurrency(); 
    std::vector<std::thread> threads;

    auto setup_worker = [&](int thread_id) {
        int64_t start, end;
        get_chunk_indices(N, num_threads, thread_id, start, end);
        for (int64_t i = start; i < end; i++) {
            A[i] = i;
        }
    };

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back(setup_worker, t);
    }

    // --- Join Threads ---
    // Wait for all setup threads to finish
    for (auto& th : threads) {
        th.join();
    }
}

int64_t
sum(int64_t N, int64_t A[])
{
    // printf(" inside sum_vector perform_sum, N=%lld \n", N);
   
    const int num_threads = std::thread::hardware_concurrency(); // 128
    std::vector<std::thread> threads;
    
    // 1. CREATE STORAGE FOR PARTIAL SUMS
    // We need one slot for each thread to write its partial result.
    // Using std::vector is safe.
    std::vector<int64_t> partial_sums(num_threads, 0);

    // This is the lambda function each thread will run
    auto sum_worker = [&](int thread_id) {
        int64_t start, end;
        get_chunk_indices(N, num_threads, thread_id, start, end);

        // 2. WRITE OUT THE LOOP (per-thread)
        // Each thread calculates its own private_sum
        int64_t private_sum = 0;
        for (int64_t i = start; i < end; i++) {
            private_sum += A[i];
        }
        
        // 3. STORE PARTIAL RESULT
        // Each thread writes to its own unique, safe spot in the vector
        partial_sums[thread_id] = private_sum;
    };

    // --- Launch Threads ---
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back(sum_worker, t);
    }

    // --- Join Threads ---
    // Wait for all threads to finish their sum
    for (auto& th : threads) {
        th.join();
    }

    // 4. SEPARATE ADDING RESULTS (Final Reduction)
    // Now that all threads are done, the main thread will safely
    // add up all the partial sums.
    int64_t total_sum = 0;
    for (int64_t partial : partial_sums) {
        total_sum += partial;
    }

    return total_sum;
}
