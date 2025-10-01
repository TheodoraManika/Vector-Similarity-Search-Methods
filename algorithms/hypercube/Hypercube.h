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

template <typename T>
class Hypercube {
    private:
        const uint32_t k; // number of hash functions that we are going to use -> size of hypercube
        uint64_t m; // constant that denotes an operation in the hash fn. Typically 2^32 - 5
        uint32_t threshold; // max elements to be checked during the algorithm
        uint32_t max_probes; // max probes of the hypercube to be checked during search
        uint32_t n_points; // number of points to be hashed initially
        uint32_t w; // a number significantly larger than the radius, affects range search
        uint32_t space_dim; // the dimension of the space we are going to work with
        uint32_t n_buckets; // number of buckets in the hash table

        // the hypercube hash table for storing vector indices
        std::unique_ptr<HashTable<int>> hypercube;

        // vector of the vectors given to us, stored here for convenience
        std::vector<std::vector<T>> feature_vectors;   

        // a list of the hash functions for every point in the hypercube
        std::list<lsh::HashFunction> hash_fns;
    
        // mapping the flipped-coins decision for every possible outcome of the hash function
        std::vector<std::unordered_map<int, bool>> flipped_coins;

    public:
        // Class constructor
        Hypercube(const uint32_t k, uint64_t m, uint32_t threshold, uint32_t max_probes, 
                  uint32_t n_points, uint32_t w, uint32_t space_dim, std::vector<std::vector<T>> init_vectors);

        ~Hypercube();

        // hash to the hypercube using the procedure from the theory
        int hash_to_hypercube(std::vector<T> vec);

        std::pair<int,T> NearestNeighbour(std::vector<T> query_vector);

        std::list<std::pair<int,T>> kNearestNeighbour(std::vector<T> query_vector, int k);

        std::list<std::pair<int,T>> RangeSearch(std::vector<T> query_vector, double radius, int c);
};
