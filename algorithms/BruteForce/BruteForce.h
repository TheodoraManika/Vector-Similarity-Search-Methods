#pragma once

#include <vector>
#include <list>
#include <utility>
#include <climits>
#include <algorithm>
#include "../../metrics/metrics.h"

/**
 * Brute Force Algorithm for Nearest Neighbor Search
 * 
 * Computes exact nearest neighbors by comparing distances to all points in the dataset.
 * Used as ground truth for evaluating approximate methods (LSH, Hypercube, etc.)
 */
template <typename T>
class BruteForce {
private:
    const uint32_t n_points;      // Number of points in the dataset
    const uint32_t space_dim;     // Dimension of the space
    std::vector<std::vector<T>> feature_vectors;  // Dataset vectors

public:
    /**
     * Constructor
     * @param n_points Number of points in dataset
     * @param space_dim Dimension of vectors
     * @param init_vectors The dataset vectors
     */
    BruteForce(uint32_t n_points, uint32_t space_dim, std::vector<std::vector<T>> init_vectors);
    
    ~BruteForce();
    
    /**
     * Find the single nearest neighbor to a query vector
     * @param query_vector The query vector
     * @return Pair of (vector_index, distance)
     */
    std::pair<int, T> NearestNeighbor(const std::vector<T>& query_vector);
    
    /**
     * Find k nearest neighbors to a query vector
     * @param query_vector The query vector
     * @param k Number of neighbors to find
     * @return Sorted list of (vector_index, distance) pairs, closest first
     */
    std::list<std::pair<int, T>> kNearestNeighbors(const std::vector<T>& query_vector, int k);
    
    /**
     * Find all neighbors within a given radius
     * @param query_vector The query vector
     * @param radius Maximum distance
     * @return List of (vector_index, distance) pairs within radius
     */
    std::list<std::pair<int, T>> RangeSearch(const std::vector<T>& query_vector, double radius);
};
