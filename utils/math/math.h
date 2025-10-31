#pragma once

#include <vector>
#include <algorithm>
#include <functional>
#include <bitset>
#include <list>
#include <cmath>
#include <random>
#include <numeric>
#include <stdexcept>

/** 
 Mathematical functions that are useful for our project
 and C++ does not offer (or offers under a different definition) 
 **/

namespace our_math {
    // Convert a number from big endian to little endian
    int big_to_little_endian(int i);
    
    // find next prime number >= n (for hash table sizing)
    uint32_t next_prime(uint32_t n);

    // Hamming distance between two 32-bit integers (e.g., hypercube vertices)
    uint32_t hamming_distance(uint32_t a, uint32_t b);
    
    // Compute optimal w parameter for LSH/Hypercube based on dataset statistics
    // Samples random pairs from dataset and computes average distance
    template <typename T>
    double compute_w_value(const std::vector<std::vector<T>>& dataset, size_t sample_size = 1000);
    
    // LSH hash function structures for Euclidean space
    // Basic hash: h(p) = floor((p·v + t) / w) where:
    //   - v ~ N(0,1)^d (d-vector with i.i.d. standard normal coordinates)
    //     Each coordinate vi ~ N(μ=0, σ²=1) for i=1,2,...,d
    //     Standard normal: mean μ=0, variance σ²=1, standard deviation σ=1
    //   - t ∈ [0, w) uniformly at random
    //   - w is the "window" parameter (typically w ∈ [2, 6])
    //
    // Amplified hash (classic with double modulo):
    //   g(p) = [(r1*h1(p) + r2*h2(p) + ... + rk*hk(p)) mod M] mod TableSize
    //   - M = 2^32 - 5 = 4294967291 (prime for good hashing)
    //   - ri ∈ Z are random int coefficients
    //   - TableSize = n/4, n/8, or n/16 (heuristic to minimize empty buckets)
    //   - Uses modular arithmetic: (a□b) mod m = ((a mod m)□(b mod m)) mod m
    namespace lsh {
        struct HashFunction {
            std::vector<double> a;  // Random vector v ~ N(0,1)^d, each vi ~ N(μ=0, σ²=1)
            double t;               // Random shift t ∈ [0, w)
        };
        
        struct BasicHashFunction {
            std::vector<double> a;  // Random vector v ~ N(0,1)^d for hi, each vi ~ N(μ=0, σ²=1)
            double t;               // Random shift t ∈ [0, w) for hi
            uint32_t r;             // Random coefficient ri for linear combination
        };
        
        struct AmplifiedHashFunction {
            std::vector<BasicHashFunction> hash_functions;  // k basic hash functions
            // Implements classic double modulo:
            // g(p) = [(r1*h1(p) + ... + rk*hk(p)) mod M] mod TableSize
            // where M = 2^32 - 5 (prime)
        };
    }
    
    // LSH hash computation functions
    template <typename T>
    int computeBasicHash(const std::vector<T>& vector, const lsh::HashFunction& hash_fn, uint32_t w);
    
    // Compute ID(p) = (r1*h1(p) + r2*h2(p) + ... + rk*hk(p)) mod M
    // ID is locality-sensitive and used for efficient filtering
    template <typename T>
    uint64_t computeID(const std::vector<T>& vector, const lsh::AmplifiedHashFunction& amp_hash_fn, uint32_t w);
    
    template <typename T>
    uint32_t computeAmplifiedHash(const std::vector<T>& vector, const lsh::AmplifiedHashFunction& amp_hash_fn, 
                                  uint32_t w, uint32_t table_size);
}