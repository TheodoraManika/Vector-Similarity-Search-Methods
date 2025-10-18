#pragma once

#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include <ctime>
#include <utility>
#include <bitset>
#include <random>

#include "../../metrics/metrics.h"
#include "../../utils/math/math.h"

template <typename T>
class Hypercube {
    private:
        const uint32_t kproj; // number of hash functions (dimension of projection) = d'
        uint64_t m; // constant that denotes an operation in the hash fn. Typically 2^32 - 5
        uint32_t M; // max candidates to be checked during the algorithm
        uint32_t probes; // max probes of the hypercube to be checked during search
        uint32_t n_points; // number of points to be hashed initially
        double w; // window size for hash functions
        uint32_t space_dim; // the dimension of the space we are going to work with
        uint32_t seed; // random seed for reproducibility

        // the hypercube hash table for storing vector indices
        std::unordered_map<uint32_t, std::vector<int>> hypercube;

        // vector of the vectors given to us, stored here for convenience
        std::vector<std::vector<T>> feature_vectors;   

        // a list of the hash functions for every point in the hypercube
        std::list<our_math::lsh::HashFunction> hash_fns;
    
        // mapping the flipped-coins decision for every possible outcome of the hash function
        std::vector<std::unordered_map<int, bool>> flipped_coins;

    public:
        // Class constructor with proper parameter names matching the specification
        Hypercube(const uint32_t kproj, uint64_t m, uint32_t M, uint32_t probes, 
                  uint32_t n_points, double w, uint32_t space_dim, 
                  std::vector<std::vector<T>> init_vectors, uint32_t seed = 1);

        ~Hypercube();

        // hash to the hypercube using the procedure from the theory
        int hash_to_hypercube(std::vector<T> vec);

        // Find nearest neighbor
        std::pair<int,T> NearestNeighbour(std::vector<T> query_vector);

        // Find k nearest neighbors 
        std::list<std::pair<int,T>> kNearestNeighbour(std::vector<T> query_vector, int k);

        // Range search within radius R
        std::list<std::pair<int,T>> RangeSearch(std::vector<T> query_vector, double radius);
};
