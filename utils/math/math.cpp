#include "math.h"
#include <random>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace our_math {
    
    inline int modulo(int a, int b) {
        int r = a % b;
        return r < 0 ? r + b : r;
    }
    
    inline int modular_power(int x, int y, int p) {
        long long result = 1;
        long long base = modulo(x, p);
        
        while (y > 0) {
            if (y & 1) {
                result = (result * base) % p;
            }
            y >>= 1;
            base = (base * base) % p;
        }
        
        return static_cast<int>(result);
    }
    
    int big_to_little_endian(int i) {
        return ((i & 0xFF000000) >> 24) |
               ((i & 0x00FF0000) >> 8)  |
               ((i & 0x0000FF00) << 8)  |
               ((i & 0x000000FF) << 24);
    }
    
    uint32_t bitwise_concat(uint16_t current, uint32_t next, int k) {
        // Shift current left by k bits and OR with next
        return (static_cast<uint32_t>(current) << k) | (next & ((1U << k) - 1));
    }
    
    inline double compute_w_value(const std::vector<std::vector<double>>& feature_vectors, int n_images) {
        if (feature_vectors.empty() || n_images <= 0) {
            return 400.0; // Default value
        }
        
        std::vector<double> distances;
        
        // Sample distances between random pairs
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, std::min(n_images, static_cast<int>(feature_vectors.size())) - 1);
        
        int sample_size = std::min(1000, n_images * n_images / 4); // Sample up to 1000 pairs
        
        for (int i = 0; i < sample_size; ++i) {
            size_t idx1 = static_cast<size_t>(dis(gen));
            size_t idx2 = static_cast<size_t>(dis(gen));
            
            if (idx1 != idx2 && idx1 < feature_vectors.size() && idx2 < feature_vectors.size()) {
                // Use the metrics from the separate metrics file for distance calculation
                // This will be computed using the metrics::EuclideanDistance function
                double dist = 0.0;
                for (size_t j = 0; j < feature_vectors[idx1].size(); ++j) {
                    double diff = feature_vectors[idx1][j] - feature_vectors[idx2][j];
                    dist += diff * diff;
                }
                dist = std::sqrt(dist);
                distances.push_back(dist);
            }
        }
        
        if (distances.empty()) {
            return 400.0;
        }
        
        // Use median distance as basis for w
        double median_dist = median(distances);
        return median_dist * 4.0; // Empirical factor
    }
    
    inline std::list<int> hamming_distance_1(int number, int k) {
        std::list<int> neighbors;
        
        for (int i = 0; i < k; ++i) {
            int neighbor = number ^ (1 << i); // Flip bit at position i
            neighbors.push_back(neighbor);
        }
        
        return neighbors;
    }
    
    inline int hamming_distance(int a, int b) {
        int xor_result = a ^ b;
        int distance = 0;
        
        while (xor_result) {
            distance += xor_result & 1;
            xor_result >>= 1;
        }
        
        return distance;
    }
    
    inline float random_float(float min, float max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(min, max);
        return dis(gen);
    }
    
    // Generate random value from Gaussian (normal) distribution
    // Standard normal when mean=0, stddev=1: N(μ=0, σ=1) with variance σ²=1
    inline double random_gaussian(double mean, double stddev) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::normal_distribution<double> dis(mean, stddev);
        return dis(gen);
    }
    
    uint32_t next_prime(uint32_t n) {
        if (n <= 2) return 2;
        if (n == 3) return 3;
        
        // Make odd
        if (n % 2 == 0) n++;
        
        auto is_prime = [](uint32_t num) {
            if (num < 2) return false;
            if (num == 2 || num == 3) return true;
            if (num % 2 == 0 || num % 3 == 0) return false;
            
            for (uint32_t i = 5; i * i <= num; i += 6) {
                if (num % i == 0 || num % (i + 2) == 0) return false;
            }
            return true;
        };
        
        while (!is_prime(n)) {
            n += 2;
        }
        return n;
    }
    
    // LSH hash function implementations for Euclidean space
    template <typename T>
    int computeBasicHash(const std::vector<T>& vector, const lsh::HashFunction& hash_fn, uint32_t w) {
        if (vector.size() != hash_fn.a.size()) {
            throw std::invalid_argument("Vector and hash function dimension mismatch");
        }
        
        // Compute dot product p·v
        double dot_product = 0.0;
        for (size_t i = 0; i < vector.size(); ++i) {
            dot_product += static_cast<double>(vector[i]) * hash_fn.a[i];
        }
        
        // Apply LSH hash function: h(p) = floor((p·v + t) / w)
        // Projects p onto line of v, shifts by t, partitions into cells of length w
        return static_cast<int>(std::floor((dot_product + hash_fn.t) / w));
    }
    
    // Compute ID(p) = (r1*h1(p) + r2*h2(p) + ... + rk*hk(p)) mod M
    // ID is locality-sensitive: depends on w-length cells on the v-lines
    // Used for efficient querying: only compute distance when ID(p) = ID(q)
    template <typename T>
    uint64_t computeID(const std::vector<T>& vector, const lsh::AmplifiedHashFunction& amp_hash_fn, uint32_t w) {
        const uint64_t M = 4294967291ULL; // 2^32 - 5 (prime number)
        uint64_t hash_sum = 0;
        
        for (const auto& basic_hash_fn : amp_hash_fn.hash_functions) {
            lsh::HashFunction hash_fn;
            hash_fn.a = basic_hash_fn.a;
            hash_fn.t = basic_hash_fn.t;
            
            // Compute hi(p) - basic hash value (can be negative)
            int64_t basic_hash = computeBasicHash(vector, hash_fn, w);
            
            // Apply modular arithmetic to keep values bounded
            // First ensure non-negative: ((x mod M) + M) mod M handles negative values
            int64_t h_mod = ((basic_hash % static_cast<int64_t>(M)) + static_cast<int64_t>(M)) % static_cast<int64_t>(M);
            
            // Compute (ri * hi(p)) mod M
            uint64_t ri = basic_hash_fn.r;
            uint64_t term = (ri * static_cast<uint64_t>(h_mod)) % M;
            
            // Accumulate: hash_sum = (hash_sum + term) mod M
            hash_sum = (hash_sum + term) % M;
        }
        
        // Return ID(p) = sum mod M
        return hash_sum;
    }
    
    template <typename T>
    uint32_t computeAmplifiedHash(const std::vector<T>& vector, const lsh::AmplifiedHashFunction& amp_hash_fn, 
                                  uint32_t w, uint32_t table_size) {
        // Classic hash function with double modulo:
        // g(p) = [(r1*h1(p) + r2*h2(p) + ... + rk*hk(p)) mod M] mod TableSize
        // which simplifies to: g(p) = ID(p) mod TableSize
        
        uint64_t id = computeID(vector, amp_hash_fn, w);
        
        // Apply second modulo: g(p) = ID(p) mod TableSize
        return static_cast<uint32_t>(id % static_cast<uint64_t>(table_size));
    }
    
    // Explicit template instantiations for LSH functions
    template int computeBasicHash<float>(const std::vector<float>&, const lsh::HashFunction&, uint32_t);
    template int computeBasicHash<double>(const std::vector<double>&, const lsh::HashFunction&, uint32_t);
    template uint64_t computeID<float>(const std::vector<float>&, const lsh::AmplifiedHashFunction&, uint32_t);
    template uint64_t computeID<double>(const std::vector<double>&, const lsh::AmplifiedHashFunction&, uint32_t);
    template uint32_t computeAmplifiedHash<float>(const std::vector<float>&, const lsh::AmplifiedHashFunction&, uint32_t, uint32_t);
    template uint32_t computeAmplifiedHash<double>(const std::vector<double>&, const lsh::AmplifiedHashFunction&, uint32_t, uint32_t);
}