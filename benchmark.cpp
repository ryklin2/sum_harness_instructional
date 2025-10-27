//
// (C) 2022-2023, E. Wes Bethel
// benchmark-* harness for running different versions of the sum study
//    over different problem sizes
//
// usage: no command line arguments
// set problem sizes, block sizes in the code below

#include <algorithm>
#include <chrono> // Make sure chrono is included
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
#include <string.h>

#include "sums.h"

/* The benchmarking program */
int main(int argc, char** argv) 
{
   std::cout << std::fixed << std::setprecision(2);

#define MAX_PROBLEM_SIZE 1 << 28  //  256M
   std::vector<int64_t> problem_sizes{ MAX_PROBLEM_SIZE >> 5, MAX_PROBLEM_SIZE >> 4, MAX_PROBLEM_SIZE >> 3, MAX_PROBLEM_SIZE >> 2, MAX_PROBLEM_SIZE >> 1, MAX_PROBLEM_SIZE};
   
   int64_t *A = (int64_t *)malloc(sizeof(int64_t) * MAX_PROBLEM_SIZE);

   int n_problems = problem_sizes.size();

   // --- PRINT A CLEAN CSV HEADER ---
   printf("Problem_Size_N,Sum_Result,Time_sec\n");

   /* For each test size */
   for (int64_t n : problem_sizes) 
   {
      int64_t t;
      // printf("Working on problem size N=%lld \n", n); // Quieted this line

      // invoke user code to set up the problem
      setup(n, &A[0]);

      // --- START TIMER ---
      auto start = std::chrono::high_resolution_clock::now();

      // invoke method to perform the sum
      t = sum(n, &A[0]);

      // --- END TIMER ---
      auto end = std::chrono::high_resolution_clock::now();
      
      // Calculate elapsed time in seconds
      std::chrono::duration<double> elapsed_seconds = end - start;
      double time_sec = elapsed_seconds.count();

      // --- PRINT THE CSV DATA ROW ---
      // This prints: N, Sum, Time
      printf("%lld,%lld,%.6f\n", n, t, time_sec);

   } // end loop over problem sizes

   free(A);
}

// EOF
