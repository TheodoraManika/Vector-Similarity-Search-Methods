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
        uint64_t m; // constant that denotes an operation in the hash fn. Typically 2^32 - 5
        uint32_t M; // size of each hash table
        uint32_t n_points; // number of points to be hashed initially
        uint32_t k; // given from the user, affects number of the buckets
        uint32_t w; // a number significantly larger than the radius, affects range search
        uint32_t space_dim; // the dimension of the space we are going to work with
        
        // a list of the hash functions for every hash table
        std::list<our_math::lsh::AmplifiedHashFunction> amplified_hash_fns;

        // vector of hash tables used by LSH. Each table stores vector indices
        std::vector<std::unordered_map<uint32_t, std::vector<int>>> lsh_tables;

        // vector of the vectors given to us, stored here for convenience
        std::vector<std::vector<T>> feature_vectors;

    public:
        // constructor of the LSH class
        LSH(uint32_t L, uint64_t m, uint32_t M,
            uint32_t n_points, uint32_t k,
            uint32_t space_dim, uint32_t w, std::vector<std::vector<T>> init_vectors);
            
        // default destructor of the LSH class
        ~LSH();

        // Search functions for LSH
        
        // function that implements NearestNeighbour
        std::pair<int,T> NearestNeighbour(const std::vector<T>& query_vector);
        
        // return a sorted list of the k nearest neighbours of a vector
        std::list<std::pair<int,T>> kNearestNeighbour(const std::vector<T>& query_vector, int k);

        // return a list of all the vectors near the query, in range (radius * c)
        std::list<std::pair<int,T>> RangeSearch(const std::vector<T>& query_vector, double radius, int c);
        
        // Helper functions
    private:
        // Validate input parameters
        void validateInput(const std::vector<T>& query_vector) const;
        
        // Check if LSH is properly initialized
        bool isInitialized() const;
        
        // Get statistics about hash table distribution
        void printStatistics() const;
};
