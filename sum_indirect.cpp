#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
#include <string.h>
#include <stdlib.h> // For lrand48() and srand48()
#include <thread>   // Use C++ Standard Threads
// could also be using random and ctime from cpp instead

#include "sums.h"

static std::vector<int64_t> data_A;

// Helper function to determine chunking (copied from sum_vector.cpp)
void get_chunk_indices(int64_t N, int num_threads, int thread_id, int64_t& start, int64_t& end) {
    // Use integer division to distribute work as evenly as possible
    // This formula also handles cases where N is not perfectly divisible by num_threads
    int64_t chunk_size = (N + num_threads - 1) / num_threads;
    start = thread_id * chunk_size;
    end = std::min(start + chunk_size, N);
}


void 
setup(int64_t N, int64_t P[]) // 'P' is our index array
{
    // printf(" inside sum_indirect problem_setup, N=%lld \n", N);
    
    srand48(1); 

    //This part was changed by Gemini and Claude when I asked for proofreading and my own research into documentation agrees.
    // We do this in a single thread because lrand48() is not thread-safe.
    // This is part of setup, so performance isn't critical.
    for (int64_t i = 0; i < N; i++) {
        P[i] = lrand48() % N;
    }

    if (data_A.size() != N) {
        data_A.resize(N);
    }


    // optimization to ensure data is NUMA-local to the cores that will use it.
    const int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    auto setup_data_worker = [&](int thread_id) {
        int64_t start, end;
        get_chunk_indices(N, num_threads, thread_id, start, end);
        
        for (int64_t i = start; i < end; i++) {
            data_A[i] = i; // value is 0..N-1, same as sum_vector
        }
    };

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back(setup_data_worker, t);
    }

    for (auto& th : threads) {
        th.join();
    }
}

int64_t
sum(int64_t N, int64_t P[]) // P is our index array
{
    // printf(" inside sum_indirect perform_sum, N=%lld \n", N);
    
    const int num_threads = std::thread::hardware_concurrency(); 
    std::vector<std::thread> threads;
    
    std::vector<int64_t> partial_sums(num_threads, 0);

    auto sum_worker = [&](int thread_id) {
        int64_t start, end;
        get_chunk_indices(N, num_threads, thread_id, start, end);

        int64_t private_sum = 0;
        
        for (int64_t i = start; i < end; i++) {
            private_sum += data_A[ P[i] ];
        }
        
        partial_sums[thread_id] = private_sum;
    };

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back(sum_worker, t);
    }

    for (auto& th : threads) {
        th.join();
    }

    int64_t total_sum = 0;
    for (int64_t partial : partial_sums) {
        total_sum += partial;
    }

    return total_sum;
}
