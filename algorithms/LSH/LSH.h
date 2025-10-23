#pragma once

#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include <ctime>
#include <utility>
#include <memory>
#include "../../utils/math/math.h"
#include "../../utils/helpers/Utils.h"

#include "../../metrics/metrics.h" 

template <typename T>
class LSH {
    private:
        uint32_t L; // number of hash tables used in LSH
        uint64_t m; // prime M for double modulo: 2^32 - 5 = 4294967291 (not directly used, hardcoded in hash)
        uint32_t M; // TableSize: size of each hash table (heuristic: n/4, n/8, or n/16)
        uint32_t n_points; // number of points to be hashed initially
        uint32_t k; // given from the user, affects number of the buckets
        uint32_t space_dim; // the dimension of the space we are going to work with
        uint32_t w; // a number significantly larger than the radius, affects range search
        
        // vector of the vectors given to us, stored here for convenience
        std::vector<std::vector<T>> feature_vectors;
        
        // LSH sensitivity parameters (r1, r2, P1, P2)-sensitive
        double r1; // Near distance threshold
        double r2; // Far distance threshold
        double P1; // Collision probability for near points (dist <= r1)
        double P2; // Collision probability for far points (dist >= r2)
        
        // a list of the hash functions for every hash table
        std::list<our_math::lsh::AmplifiedHashFunction> amplified_hash_fns;

        // vector of hash tables used by LSH. Each table stores (index, ID) pairs
        // ID = sum(ri*hi(p)) mod M enables efficient querying: only compute distance when ID(p) = ID(q)
        std::vector<std::unordered_map<uint32_t, std::vector<std::pair<int, uint64_t>>>> lsh_tables;

    public:
        // constructor of the LSH class
        LSH(uint32_t L, uint64_t m, uint32_t M,
            uint32_t n_points, uint32_t k,
            uint32_t space_dim, uint32_t w, std::vector<std::vector<T>> init_vectors,
            int seed = 1);
        
        // Constructor with explicit (r1, r2, P1, P2)-sensitivity parameters
        LSH(double r1, double r2, double P1, double P2,
            uint32_t space_dim, uint32_t w,
            std::vector<std::vector<T>> init_vectors,
            double target_success_prob = 0.9,
            int seed = 1);
            
        // default destructor of the LSH class
        ~LSH();

        // Search functions for LSH
        
        // function that implements NearestNeighbour
        std::pair<int,T> NearestNeighbour(const std::vector<T>& query_vector);
        
        // return a sorted list of the k nearest neighbours of a vector
        std::list<std::pair<int,T>> kNearestNeighbour(const std::vector<T>& query_vector, int k);

        // return a list of all the vectors near the query, in range (radius * c)
        std::list<std::pair<int,T>> RangeSearch(const std::vector<T>& query_vector, double radius, int c);
        
        // Get LSH sensitivity parameters
        double getR1() const { return r1; }
        double getR2() const { return r2; }
        double getP1() const { return P1; }
        double getP2() const { return P2; }
        
        // Verify LSH family collision probabilities empirically
        void verifyLSHProperty(const std::vector<std::vector<T>>& sample_points,
                              size_t num_samples = 1000) const;
        
        // Helper functions
    private:
        // Compute optimal k and L based on P1, P2
        static std::pair<uint32_t, uint32_t> computeOptimalParameters(
            double P1, double P2, double target_success_prob = 0.9);
        // Validate input parameters
        void validateInput(const std::vector<T>& query_vector) const;
        
        // Check if LSH is properly initialized
        bool isInitialized() const;
        
        // Get statistics about hash table distribution
        void printStatistics() const;
};
