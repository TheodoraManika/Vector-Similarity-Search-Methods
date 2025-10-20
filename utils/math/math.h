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
    // modulo that works for every integer number
    int modulo(int a, int b);

    // executes the operation (x^y) mod p
    int modular_power(int x, int y, int p);
    
    // Convert a number from big endian to little endian
    int big_to_little_endian(int i);

    // Bitwise concatenate 2 integers
    uint32_t bitwise_concat(uint16_t current, uint32_t next, int k);
    
    // compute the best w value
    inline double compute_w_value(const std::vector<std::vector<double>>& feature_vectors, int n_images);

    // find all of the numbers that have hamming distance = 1 with a given number of bitwise size k
    inline std::list<int> hamming_distance_1(int number, int k);

    // find the hamming distance between 2 integers
    inline int hamming_distance(int a, int b);
    
    template <typename T>
    // add the vector b to a
    inline std::vector<T> add_vectors(const std::vector<T>& a, const std::vector<T>& b);

    template <typename T>
    // divide a vector by an integer
    inline std::vector<T> divide(const std::vector<T>& a, int k);

    template <typename T>
    // find the median member of a vector
    inline T median(std::vector<T> a);
    
    // generate random float in range [min, max]
    inline float random_float(float min, float max);
    
    // generate random gaussian (normal) distributed value
    // Standard normal: N(μ=0, σ=1) with mean=0, variance σ²=1, standard deviation σ=1
    inline double random_gaussian(double mean = 0.0, double stddev = 1.0);
    
    // find next prime number >= n (for hash table sizing)
    inline uint32_t next_prime(uint32_t n);
    
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
    
    // Implementation of template functions
    template <typename T>
    inline std::vector<T> add_vectors(const std::vector<T>& a, const std::vector<T>& b) {
        if (a.size() != b.size()) {
            throw std::invalid_argument("Vectors must have same dimension");
        }
        std::vector<T> result(a.size());
        for (size_t i = 0; i < a.size(); ++i) {
            result[i] = a[i] + b[i];
        }
        return result;
    }
    
    template <typename T>
    inline std::vector<T> divide(const std::vector<T>& a, int k) {
        std::vector<T> result(a.size());
        for (size_t i = 0; i < a.size(); ++i) {
            result[i] = a[i] / static_cast<T>(k);
        }
        return result;
    }
    
    template <typename T>
    inline T median(std::vector<T> a) {
        std::sort(a.begin(), a.end());
        size_t n = a.size();
        if (n % 2 == 0) {
            return (a[n/2 - 1] + a[n/2]) / 2;
        } else {
            return a[n/2];
        }
    }
}