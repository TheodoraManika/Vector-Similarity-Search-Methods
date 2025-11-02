#pragma once

#include <iostream>
#include <list>
#include <unordered_map>
#include <vector>

#include "../../metrics/metrics.h"
#include "../../utils/math/math.h"

template <typename T>
class Hypercube {
    private:
        uint32_t kproj;
        uint64_t m;
        uint32_t M;          // max points to check 
        uint32_t probes;     // max vertices to probe
        uint32_t n_points;
        double w;
        uint32_t space_dim;
        uint32_t seed;

        uint32_t n_buckets;  // NEW: 2^kproj

        // change storage to dense 2^kproj table of lists (legacy style)
        std::vector<std::list<int>> hypercube;  // NEW

        // list of hash functions (unchanged type)
        std::list<our_math::lsh::HashFunction> hash_fns;

        // pre-mapped f coins per hash function for all bucket indices [0..n_buckets-1]
        std::vector<std::vector<bool>> flipped_coins;

        std::vector<std::vector<T>> feature_vectors;

        // Helper: generate all vertices at exact Hamming distance from a given vertex
        std::vector<uint32_t> neighbors_by_hamming(uint32_t vertex, uint32_t dist) const;

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

        // add legacy-style RangeSearch with scale c
        std::list<std::pair<int,T>> RangeSearch(std::vector<T> query_vector, double radius, int c);
        // keep your existing API, forward to the legacy one with c=1
        std::list<std::pair<int,T>> RangeSearch(std::vector<T> query_vector, double radius);
};
