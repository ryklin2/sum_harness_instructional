#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
#include <string.h>
#include <thread> //I hope C++ standard threads library is allowed... I guess I could do it a bit more manually.
//I'm not sure if how this applies to like OMP and such either, but I figureed a really standard library is probably okay.
//If it was really needed I guess I could write out like 128 threads manually but that seems excessive.

#include "sums.h"

static std::vector<int64_t> data_A;

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
    // Google Gemini suggested emplace_back while I asked it to proofread and I like this function so I'm using it.
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back(setup_worker, t);
    }

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
    std::vector<int64_t> partial_sums(num_threads, 0);

    auto sum_worker = [&](int thread_id) {
        int64_t start, end;
        get_chunk_indices(N, num_threads, thread_id, start, end);

        int64_t private_sum = 0;
        for (int64_t i = start; i < end; i++) {
            private_sum += A[i];
        }
        // each thread writes to its own unique, safe spot in the vector
        partial_sums[thread_id] = private_sum;
    };
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back(sum_worker, t);
    }

    // wait for all threads to finish their sum
    for (auto& th : threads) {
        th.join();
    }

    // add up all the partial sums.
    int64_t total_sum = 0;
    for (int64_t partial : partial_sums) {
        total_sum += partial;
    }

    return total_sum;
}
