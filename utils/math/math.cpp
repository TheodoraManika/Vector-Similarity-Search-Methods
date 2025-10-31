#include "math.h"
#include <cmath>

namespace our_math {
    
    int big_to_little_endian(int i) {
        return ((i & 0xFF000000) >> 24) |
               ((i & 0x00FF0000) >> 8)  |
               ((i & 0x0000FF00) << 8)  |
               ((i & 0x000000FF) << 24);
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

    // Hamming distance between two 32-bit integers (vertex ids)
    uint32_t hamming_distance(uint32_t a, uint32_t b) {
    uint32_t x = a ^ b;
#if defined(__GNUG__) || defined(__clang__)
    return static_cast<uint32_t>(__builtin_popcount(x));
#else
    uint32_t cnt = 0;
    while (x) { x &= (x - 1); ++cnt; }
    return cnt;
#endif
    }

    // Hamming distance between two sequences (counts mismatches). Assumes same size.
    template <typename T>
    inline std::size_t hamming_distance(const std::vector<T>& a, const std::vector<T>& b) {
        std::size_t n = a.size();
        std::size_t cnt = 0;
        for (std::size_t i = 0; i < n; ++i) {
            if (a[i] != b[i]) ++cnt;
        }
        return cnt;
    }
    
    // Compute optimal w parameter for LSH/Hypercube based on dataset statistics
    // Sample random pairs and compute their average Manhattan distance
    // This helps set the "bucket width" appropriately for the data distribution
    template <typename T>
    double compute_w_value(const std::vector<std::vector<T>>& dataset, size_t sample_size) {
        if (dataset.empty() || dataset[0].empty()) {
            return 4.0; // fallback default
        }
        
        size_t n = dataset.size();
        size_t dim = dataset[0].size();
        
        // Limit sample size to dataset size
        size_t actual_samples = std::min(sample_size, n * (n - 1) / 2);
        if (actual_samples == 0) return 4.0;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dist(0, n - 1);
        
        double sum_distances = 0.0;
        size_t valid_samples = 0;
        
        for (size_t s = 0; s < actual_samples; ++s) {
            // Sample two different random vectors
            size_t i = dist(gen);
            size_t j = dist(gen);
            
            // Ensure they're different
            while (i == j && n > 1) {
                j = dist(gen);
            }
            
            // Compute Manhattan distance
            double dist_sum = 0.0;
            for (size_t d = 0; d < dim; ++d) {
                dist_sum += std::abs(static_cast<double>(dataset[i][d]) - static_cast<double>(dataset[j][d]));
            }
            
            sum_distances += dist_sum;
            valid_samples++;
        }
        
        if (valid_samples == 0) return 4.0;
        
        // Return average distance (this will be scaled by caller, e.g., w = 20 * avg)
        return sum_distances / valid_samples;
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
    
    // Template instantiations for compute_w_value
    template double compute_w_value<float>(const std::vector<std::vector<float>>&, size_t);
    template double compute_w_value<double>(const std::vector<std::vector<double>>&, size_t);
}