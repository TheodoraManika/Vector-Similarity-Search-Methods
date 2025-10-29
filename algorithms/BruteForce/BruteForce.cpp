#include "BruteForce.h"
#include <iostream>

template <typename T>
BruteForce<T>::BruteForce(uint32_t n_points, uint32_t space_dim, std::vector<std::vector<T>> init_vectors)
    : n_points(n_points), space_dim(space_dim), feature_vectors(init_vectors) {
}

template <typename T>
BruteForce<T>::~BruteForce() {
}

template <typename T>
std::pair<int, T> BruteForce<T>::NearestNeighbor(const std::vector<T>& query_vector) {
    T min_distance = static_cast<T>(INT_MAX);
    int result_vector_index = -1;
    
    // Check distance to all points
    for (uint32_t i = 0; i < n_points; ++i) {
        T distance = metrics::ManhattanDistance(feature_vectors[i], query_vector, space_dim);
        
        if (distance < min_distance) {
            min_distance = distance;
            result_vector_index = i;
        }
    }
    
    return std::make_pair(result_vector_index, min_distance);
}

template <typename T>
std::list<std::pair<int, T>> BruteForce<T>::kNearestNeighbors(const std::vector<T>& query_vector, int k) {
    std::list<std::pair<int, T>> result;
    T kth_min_distance = static_cast<T>(INT_MAX);
    
    // Check all points in the dataset
    for (uint32_t i = 0; i < n_points; ++i) {
        T distance = metrics::ManhattanDistance(feature_vectors[i], query_vector, space_dim);
        std::pair<int, T> new_pair = std::make_pair(i, distance);
        
        // Insert into sorted list if appropriate
        if (result.size() > 0 && result.size() < static_cast<size_t>(k)) {
            // List has space, insert in sorted order
            if (distance >= kth_min_distance) {
                // Insert at back if distance >= current max
                kth_min_distance = distance;
                result.push_back(new_pair);
            } else {
                // Insert in sorted position
                auto it = result.begin();
                for (; it != result.end(); ++it) {
                    if (distance < it->second) {
                        result.insert(it, new_pair);
                        break;
                    }
                }
            }
        } else if (result.size() > 0 && result.size() == static_cast<size_t>(k)) {
            // List is full, only insert if better than worst
            if (distance < kth_min_distance) {
                // Find insertion point
                auto it = result.begin();
                for (; it != result.end(); ++it) {
                    if (distance < it->second) {
                        result.insert(it, new_pair);
                        break;
                    }
                }
                // Remove worst element
                result.pop_back();
                kth_min_distance = result.back().second;
            }
        } else if (result.size() == 0) {
            // First element
            kth_min_distance = distance;
            result.push_back(new_pair);
        }
    }
    
    return result;
}

template <typename T>
std::list<std::pair<int, T>> BruteForce<T>::RangeSearch(const std::vector<T>& query_vector, double radius) {
    std::list<std::pair<int, T>> result;
    
    // Check all points in the dataset
    for (uint32_t i = 0; i < n_points; ++i) {
        T distance = metrics::ManhattanDistance(feature_vectors[i], query_vector, space_dim);
        
        if (distance <= static_cast<T>(radius)) {
            result.emplace_back(i, distance);
        }
    }
    
    return result;
}

// Explicit template instantiations
template class BruteForce<float>;
template class BruteForce<double>;
template class BruteForce<int>;
