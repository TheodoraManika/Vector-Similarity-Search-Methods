#include "Hypercube.h"
#include "../../utils/math/math.h"
#include <algorithm>
#include <queue>
#include <unordered_set>
#include <cmath>
#include <random>

template <typename T>
Hypercube<T>::Hypercube(const uint32_t kproj, uint64_t m, uint32_t M, uint32_t probes, 
                        uint32_t n_points, double w, uint32_t space_dim, 
                        std::vector<std::vector<T>> init_vectors, uint32_t seed)
    : kproj(kproj), m(m), M(M), probes(probes), n_points(n_points), w(w), 
      space_dim(space_dim), seed(seed), feature_vectors(init_vectors) {
    
    // Initialize random number generator with seed
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::normal_distribution<double> gauss(0.0, 1.0);
    
    // Initialize hash functions
    for (uint32_t i = 0; i < kproj; ++i) {
        our_math::lsh::HashFunction hash_fn;
        hash_fn.a.resize(space_dim);
        
        // Initialize coefficients from Gaussian distribution
        for (uint32_t d = 0; d < space_dim; ++d) {
            hash_fn.a[d] = gauss(gen);
        }
        
        // Random shift
        hash_fn.t = dis(gen) * w;
        
        hash_fns.push_back(hash_fn);
    }
    
    // Initialize flipped coins mappings
    flipped_coins.resize(kproj);
    
    // Hash all initial vectors
    for (size_t vec_idx = 0; vec_idx < feature_vectors.size(); ++vec_idx) {
        uint32_t bucket = hash_to_hypercube(feature_vectors[vec_idx]);
        hypercube[bucket].push_back(static_cast<int>(vec_idx));
    }
}

template <typename T>
Hypercube<T>::~Hypercube() {
}

template <typename T>
int Hypercube<T>::hash_to_hypercube(std::vector<T> vec) {
    uint32_t result = 0;
    uint32_t bit_position = 0;
    
    auto hash_it = hash_fns.begin();
    for (uint32_t i = 0; i < kproj; ++i, ++hash_it) {
        int hash_val = our_math::computeBasicHash(vec, *hash_it, static_cast<uint32_t>(w));
        
        // Check flipped coin mapping
        bool bit_value;
        if (flipped_coins[i].find(hash_val) != flipped_coins[i].end()) {
            bit_value = flipped_coins[i][hash_val];
        } else {
            // First time seeing this hash value, flip a coin
            std::mt19937 gen(seed + hash_val + i);
            std::uniform_int_distribution<> dis(0, 1);
            bit_value = dis(gen) == 1;
            flipped_coins[i][hash_val] = bit_value;
        }
        
        if (bit_value) {
            result |= (1U << bit_position);
        }
        bit_position++;
    }
    
    return static_cast<int>(result);
}

template <typename T>
std::pair<int,T> Hypercube<T>::NearestNeighbour(std::vector<T> query_vector) {
    auto neighbors = kNearestNeighbour(query_vector, 1);
    if (neighbors.empty()) {
        return std::make_pair(-1, std::numeric_limits<T>::max());
    }
    return neighbors.front();
}

template <typename T>
std::list<std::pair<int,T>> Hypercube<T>::kNearestNeighbour(std::vector<T> query_vector, int k) {
    // Priority queue to store k nearest neighbors (max heap)
    std::priority_queue<std::pair<T, int>> pq;
    std::unordered_set<int> visited_vertices;
    std::unordered_set<int> checked_points;
    
    uint32_t query_bucket = hash_to_hypercube(query_vector);
    
    // Queue for BFS on hypercube vertices
    std::queue<uint32_t> vertex_queue;
    vertex_queue.push(query_bucket);
    visited_vertices.insert(query_bucket);
    
    uint32_t probes_checked = 0;
    uint32_t points_checked = 0;
    
    while (!vertex_queue.empty() && probes_checked < probes && points_checked < M) {
        uint32_t current_vertex = vertex_queue.front();
        vertex_queue.pop();
        probes_checked++;
        
        // Check all points in current bucket
        if (hypercube.find(current_vertex) != hypercube.end()) {
            for (int point_idx : hypercube[current_vertex]) {
                if (checked_points.find(point_idx) != checked_points.end()) {
                    continue; // Already checked this point
                }
                checked_points.insert(point_idx);
                points_checked++;
                
                if (points_checked >= M) break;
                
                T distance = metrics::ManhattanDistance(query_vector, feature_vectors[point_idx], query_vector.size());
                
                if (pq.size() < static_cast<size_t>(k)) {
                    pq.emplace(distance, point_idx);
                } else if (distance < pq.top().first) {
                    pq.pop();
                    pq.emplace(distance, point_idx);
                }
            }
        }
        
        // Add neighboring vertices (differ by 1 bit)
        for (uint32_t i = 0; i < kproj; ++i) {
            uint32_t neighbor = current_vertex ^ (1U << i);
            if (visited_vertices.find(neighbor) == visited_vertices.end()) {
                visited_vertices.insert(neighbor);
                vertex_queue.push(neighbor);
            }
        }
    }
    
    // Convert priority queue to list (reverse order for nearest first)
    std::list<std::pair<int,T>> result;
    while (!pq.empty()) {
        result.emplace_front(pq.top().second, pq.top().first);
        pq.pop();
    }
    
    return result;
}

template <typename T>
std::list<std::pair<int,T>> Hypercube<T>::RangeSearch(std::vector<T> query_vector, double radius) {
    std::list<std::pair<int,T>> result;
    std::unordered_set<int> visited_vertices;
    std::unordered_set<int> checked_points;
    
    uint32_t query_bucket = hash_to_hypercube(query_vector);
    
    // Queue for BFS on hypercube vertices
    std::queue<uint32_t> vertex_queue;
    vertex_queue.push(query_bucket);
    visited_vertices.insert(query_bucket);
    
    uint32_t probes_checked = 0;
    uint32_t points_checked = 0;
    
    while (!vertex_queue.empty() && probes_checked < probes && points_checked < M) {
        uint32_t current_vertex = vertex_queue.front();
        vertex_queue.pop();
        probes_checked++;
        
        // Check all points in current bucket
        if (hypercube.find(current_vertex) != hypercube.end()) {
            for (int point_idx : hypercube[current_vertex]) {
                if (checked_points.find(point_idx) != checked_points.end()) {
                    continue; // Already checked this point
                }
                checked_points.insert(point_idx);
                points_checked++;
                
                if (points_checked >= M) break;
                
                T distance = metrics::ManhattanDistance(query_vector, feature_vectors[point_idx], query_vector.size());
                
                if (distance <= radius) {
                    result.emplace_back(point_idx, distance);
                }
            }
        }
        
        // Add neighboring vertices (differ by 1 bit)
        for (uint32_t i = 0; i < kproj; ++i) {
            uint32_t neighbor = current_vertex ^ (1U << i);
            if (visited_vertices.find(neighbor) == visited_vertices.end()) {
                visited_vertices.insert(neighbor);
                vertex_queue.push(neighbor);
            }
        }
    }
    
    return result;
}

// Explicit template instantiation
template class Hypercube<float>;
template class Hypercube<double>;