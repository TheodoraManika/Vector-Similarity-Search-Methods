#include "IVFFlat.h"
#include <algorithm>
#include <queue>
#include <set>
#include <cassert>
#include <iostream>
#include <cmath>
#include <random>

template <typename T>
IVFFlat<T>::IVFFlat(uint32_t n_clusters, uint32_t n_probe)
    : n_clusters(n_clusters), n_probe(n_probe), dimensions(0), is_trained(false) {
    if (n_clusters == 0) {
        throw std::invalid_argument("Number of clusters must be greater than 0");
    }
    if (n_probe == 0) {
        throw std::invalid_argument("Number of probes must be greater than 0");
    }
    if (n_probe > n_clusters) {
        std::cerr << "Warning: n_probe (" << n_probe << ") > n_clusters (" << n_clusters 
                  << "), setting n_probe = n_clusters" << std::endl;
        this->n_probe = n_clusters;
    }
    inverted_lists.resize(n_clusters);
}

template <typename T>
bool IVFFlat<T>::train(const std::vector<std::vector<T>>& data) {
    if (data.empty()) {
        std::cerr << "Error: Cannot train IVFFlat with empty dataset" << std::endl;
        return false;
    }
    
    dimensions = data[0].size();
    
    if (dimensions == 0) {
        std::cerr << "Error: Vector dimensions cannot be 0" << std::endl;
        return false;
    }
    
    // Validate dimensions
    for (const auto& vec : data) {
        if (vec.size() != dimensions) {
            std::cerr << "Error: All vectors must have same dimensions" << std::endl;
            return false;
        }
    }
    
    // Validate that we have enough data points for clustering
    if (data.size() < n_clusters) {
        std::cerr << "Warning: Dataset size (" << data.size() 
                  << ") is smaller than number of clusters (" << n_clusters << ")" << std::endl;
    }
    
    std::cout << "Training IVFFlat with " << data.size() << " vectors, " 
              << dimensions << " dimensions, " << n_clusters << " clusters..." << std::endl;
    
    // Reserve space for dataset
    dataset.clear();
    dataset.reserve(data.size());
    dataset = data;
    
    // Step 1: Select random subset X' ⊂ X where |X'| ∼ √n
    size_t n = data.size();
    size_t subset_size = static_cast<size_t>(std::sqrt(n));
    
    // Ensure subset is at least as large as number of clusters
    subset_size = std::max(subset_size, static_cast<size_t>(n_clusters));
    subset_size = std::min(subset_size, n);  // Don't exceed dataset size
    
    std::cout << "Selecting random subset of size " << subset_size 
              << " (~√n = ~" << std::sqrt(n) << ") for k-means training..." << std::endl;
    
    // Create random subset X' for k-means training
    std::vector<size_t> indices(n);
    for (size_t i = 0; i < n; ++i) {
        indices[i] = i;
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);
    
    std::vector<std::vector<T>> training_subset;
    training_subset.reserve(subset_size);
    std::vector<size_t> training_indices;
    training_indices.reserve(subset_size);
    
    for (size_t i = 0; i < subset_size; ++i) {
        training_subset.push_back(data[indices[i]]);
        training_indices.push_back(indices[i]);
    }
    
    // Train k-means clustering on subset X'
    kmeans = std::make_unique<KMeans<T>>(n_clusters);
    kmeans->set_data(training_subset);
    
    if (!kmeans->fit()) {
        std::cerr << "Error: K-means clustering failed" << std::endl;
        is_trained = false;
        return false;
    }
    
    // Get centroids C = {c1, ..., ck}
    centroids = kmeans->get_centroids();
    
    if (centroids.empty() || centroids.size() != n_clusters) {
        std::cerr << "Error: Invalid centroids after k-means clustering" << std::endl;
        is_trained = false;
        return false;
    }
    
    // Clear inverted lists
    for (auto& list : inverted_lists) {
        list.clear();
    }
    
    // Step 2 & 3: Assignment for ALL vectors in X
    // For each x ∈ X, find nearest centroid j*(x) = arg min ||x - cj||²
    // and append (id(x), x) to inverted list ILj*(x)
    std::cout << "Assigning all vectors to nearest centroids..." << std::endl;
    
    for (size_t i = 0; i < data.size(); ++i) {
        // Find nearest centroid: j*(x) = arg min ||x - cj||²
        T min_distance = std::numeric_limits<T>::max();
        int nearest_cluster = 0;
        
        for (size_t j = 0; j < centroids.size(); ++j) {
            T distance = metrics::EuclideanDistance(data[i], centroids[j], dimensions);
            if (distance < min_distance) {
                min_distance = distance;
                nearest_cluster = static_cast<int>(j);
            }
        }
        
        // Append (id(x), x) to inverted list ILj*(x)
        if (nearest_cluster >= 0 && nearest_cluster < static_cast<int>(n_clusters)) {
            inverted_lists[nearest_cluster].push_back(static_cast<int>(i));
        }
    }
    
    // Print clustering statistics
    std::cout << "Clustering completed. Silhouette score: " 
              << kmeans->get_silhouette_score() << std::endl;
    
    is_trained = true;
    return true;
}

template <typename T>
void IVFFlat<T>::add(const std::vector<std::vector<T>>& vectors) {
    if (!is_trained) {
        throw std::runtime_error("Error: Index must be trained before adding vectors");
    }
    
    if (vectors.empty()) {
        return;
    }
    
    size_t start_idx = dataset.size();
    
    // Reserve space to avoid multiple reallocations
    dataset.reserve(dataset.size() + vectors.size());
    
    // Add vectors to dataset
    for (const auto& vec : vectors) {
        if (vec.size() != dimensions) {
            std::cerr << "Warning: Skipping vector with wrong dimensions (expected " 
                      << dimensions << ", got " << vec.size() << ")" << std::endl;
            continue;
        }
        dataset.push_back(vec);
    }
    
    // Assign new vectors to clusters
    for (size_t i = start_idx; i < dataset.size(); ++i) {
        int cluster_id = kmeans->predict(dataset[i]);
        if (cluster_id >= 0 && cluster_id < static_cast<int>(n_clusters)) {
            inverted_lists[cluster_id].push_back(static_cast<int>(i));
        } else {
            std::cerr << "Warning: Invalid cluster assignment for added vector" << std::endl;
        }
    }
}

template <typename T>
std::vector<int> IVFFlat<T>::find_nearest_clusters(const std::vector<T>& query, uint32_t n_clusters_to_find) const {
    std::vector<std::pair<T, int>> cluster_distances;
    cluster_distances.reserve(centroids.size());
    
    // Step 1 (IVF Query): Evaluate all cells - compute ||q - cj||² for j = 1, ..., k
    // Using squared distance for coarse search (faster, ordering preserved)
    for (size_t i = 0; i < centroids.size(); ++i) {
        T distance_squared = metrics::EuclideanDistanceSquared(query, centroids[i], dimensions);
        cluster_distances.emplace_back(distance_squared, static_cast<int>(i));
    }
    
    // Let S ⊂ {1, ..., k} be indices of best b "probe" cells
    // Sort by distance and take n_clusters_to_find (b) closest
    uint32_t num_to_select = std::min(n_clusters_to_find, static_cast<uint32_t>(cluster_distances.size()));
    std::partial_sort(cluster_distances.begin(), 
                     cluster_distances.begin() + num_to_select,
                     cluster_distances.end());
    
    std::vector<int> nearest_clusters;
    nearest_clusters.reserve(num_to_select);
    
    for (uint32_t i = 0; i < num_to_select; ++i) {
        nearest_clusters.push_back(cluster_distances[i].second);
    }
    
    return nearest_clusters;
}

template <typename T>
bool IVFFlat<T>::validate_query(const std::vector<T>& query) const {
    if (query.empty()) {
        std::cerr << "Error: Query vector is empty" << std::endl;
        return false;
    }
    
    if (query.size() != dimensions) {
        std::cerr << "Error: Query vector dimension (" << query.size() 
                  << ") does not match index dimension (" << dimensions << ")" << std::endl;
        return false;
    }
    
    // Check for NaN or infinite values
    for (const auto& val : query) {
        if (std::isnan(val) || std::isinf(val)) {
            std::cerr << "Error: Query vector contains NaN or infinite values" << std::endl;
            return false;
        }
    }
    
    return true;
}

template <typename T>
void IVFFlat<T>::set_n_probe(uint32_t new_n_probe) {
    if (new_n_probe == 0) {
        std::cerr << "Warning: n_probe cannot be 0, keeping current value" << std::endl;
        return;
    }
    if (new_n_probe > n_clusters) {
        std::cerr << "Warning: n_probe (" << new_n_probe << ") > n_clusters (" << n_clusters 
                  << "), setting n_probe = n_clusters" << std::endl;
        n_probe = n_clusters;
    } else {
        n_probe = new_n_probe;
    }
}

template <typename T>
std::pair<int, T> IVFFlat<T>::search_nearest(const std::vector<T>& query) const {
    if (!is_trained) {
        throw std::runtime_error("Error: Index must be trained before searching");
    }
    
    if (!validate_query(query)) {
        return std::make_pair(-1, std::numeric_limits<T>::max());
    }
    
    // Step 1: Find nearest b clusters to probe (coarse search)
    // Returns S ⊂ {1, ..., k} indices of best probe cells
    std::vector<int> clusters_to_probe = find_nearest_clusters(query, n_probe);
    
    int best_idx = -1;
    T best_distance = std::numeric_limits<T>::max();
    
    // Step 2: Let U = ⋃_{j∈S} ILj - union of inverted lists from probe cells
    // Compute exact d(q, x) = ||q - x||² for all x ∈ U (fine search)
    for (int cluster_id : clusters_to_probe) {
        const auto& inverted_list = inverted_lists[cluster_id];
        
        for (int vec_idx : inverted_list) {
            T distance = metrics::EuclideanDistance(query, dataset[vec_idx], dimensions);
            
            if (distance < best_distance) {
                best_distance = distance;
                best_idx = vec_idx;
            }
        }
    }
    
    // Step 3: Return nearest x ∈ U for query q
    return std::make_pair(best_idx, best_distance);
}

template <typename T>
std::list<std::pair<int, T>> IVFFlat<T>::search_k_nearest(const std::vector<T>& query, uint32_t k) const {
    if (!is_trained) {
        throw std::runtime_error("Error: Index must be trained before searching");
    }
    
    if (!validate_query(query)) {
        return std::list<std::pair<int, T>>();
    }
    
    if (k == 0) {
        return std::list<std::pair<int, T>>();
    }
    
    // Step 1: Find nearest b clusters to probe (coarse search)
    // Returns S ⊂ {1, ..., k} indices of best probe cells
    // Time complexity: O(k·d) where k = n_clusters, d = dimensions
    std::vector<int> clusters_to_probe = find_nearest_clusters(query, n_probe);
    
    // Use priority queue to maintain k best candidates (max heap by distance)
    std::priority_queue<std::pair<T, int>> pq;
    
    // Step 2: Let U = ⋃_{j∈S} ILj - union of inverted lists from probe cells
    // Compute exact d(q, x) = ||q - x||² for all x ∈ U (fine search)
    // Time complexity: O(|U|·d) where |U| = total points in probed cells
    for (int cluster_id : clusters_to_probe) {
        const auto& inverted_list = inverted_lists[cluster_id];
        
        for (int vec_idx : inverted_list) {
            T distance = metrics::EuclideanDistance(query, dataset[vec_idx], dimensions);
            
            if (pq.size() < k) {
                pq.emplace(distance, vec_idx);
            } else if (distance < pq.top().first) {
                pq.pop();
                pq.emplace(distance, vec_idx);
            }
        }
    }
    
    // Step 3: Return R nearest x ∈ U for query q (candidates in ≥ 1 cells)
    // Convert priority queue to sorted list (ascending order by distance)
    std::list<std::pair<int, T>> result;
    
    while (!pq.empty()) {
        auto& top = pq.top();
        result.emplace_front(top.second, top.first);
        pq.pop();
    }
    
    return result;
}

template <typename T>
std::list<std::pair<int, T>> IVFFlat<T>::range_search(const std::vector<T>& query, T radius) const {
    if (!is_trained) {
        throw std::runtime_error("Error: Index must be trained before searching");
    }
    
    if (!validate_query(query)) {
        return std::list<std::pair<int, T>>();
    }
    
    if (radius < 0) {
        std::cerr << "Warning: Negative radius provided, using absolute value" << std::endl;
        radius = std::abs(radius);
    }
    
    std::list<std::pair<int, T>> result;
    std::set<int> checked_indices;
    
    // Find clusters within radius + max_cluster_radius
    // For simplicity, we probe more clusters for range search
    uint32_t clusters_to_probe = std::min(n_probe * 2, n_clusters);
    std::vector<int> clusters = find_nearest_clusters(query, clusters_to_probe);
    
    // Search in selected clusters
    for (int cluster_id : clusters) {
        const auto& inverted_list = inverted_lists[cluster_id];
        
        for (int vec_idx : inverted_list) {
            // Skip if already checked
            if (checked_indices.find(vec_idx) != checked_indices.end()) {
                continue;
            }
            checked_indices.insert(vec_idx);
            
            T distance = metrics::EuclideanDistance(query, dataset[vec_idx], dimensions);
            
            if (distance <= radius) {
                result.emplace_back(vec_idx, distance);
            }
        }
    }
    
    // Sort by distance
    result.sort([](const std::pair<int, T>& a, const std::pair<int, T>& b) {
        return a.second < b.second;
    });
    
    return result;
}

template <typename T>
void IVFFlat<T>::print_index_info() const {
    std::cout << "=== IVFFlat Index Information ===" << std::endl;
    std::cout << "Trained: " << (is_trained ? "Yes" : "No") << std::endl;
    std::cout << "Number of clusters: " << n_clusters << std::endl;
    std::cout << "Number of probes: " << n_probe << std::endl;
    std::cout << "Vector dimensions: " << dimensions << std::endl;
    std::cout << "Total vectors: " << dataset.size() << std::endl;
    
    if (!inverted_lists.empty() && is_trained) {
        size_t min_size = inverted_lists[0].size();
        size_t max_size = inverted_lists[0].size();
        size_t total_size = 0;
        size_t empty_clusters = 0;
        
        for (const auto& list : inverted_lists) {
            min_size = std::min(min_size, list.size());
            max_size = std::max(max_size, list.size());
            total_size += list.size();
            if (list.empty()) {
                ++empty_clusters;
            }
        }
        
        double avg_size = static_cast<double>(total_size) / inverted_lists.size();
        std::cout << "Cluster sizes - Min: " << min_size 
                  << ", Max: " << max_size 
                  << ", Avg: " << avg_size << std::endl;
        std::cout << "Empty clusters: " << empty_clusters << std::endl;
        
        // Calculate imbalance ratio
        if (avg_size > 0) {
            double imbalance = static_cast<double>(max_size) / avg_size;
            std::cout << "Cluster imbalance ratio: " << imbalance << std::endl;
        }
    }
    
    if (kmeans && is_trained) {
        std::cout << "Silhouette score: " << kmeans->get_silhouette_score() << std::endl;
    }
    
    std::cout << "Memory usage: " << (get_memory_usage() / 1024.0 / 1024.0) << " MB" << std::endl;
    std::cout << "=================================" << std::endl;
}

template <typename T>
size_t IVFFlat<T>::get_memory_usage() const noexcept {
    size_t memory = 0;
    
    // Dataset memory
    memory += dataset.size() * dimensions * sizeof(T);
    
    // Centroids memory
    memory += centroids.size() * dimensions * sizeof(T);
    
    // Inverted lists memory (vector indices)
    for (const auto& list : inverted_lists) {
        memory += list.capacity() * sizeof(int);
    }
    
    // Inverted lists container overhead
    memory += inverted_lists.capacity() * sizeof(std::vector<int>);
    
    return memory;
}

template <typename T>
void IVFFlat<T>::clear() {
    dataset.clear();
    dataset.shrink_to_fit();
    
    centroids.clear();
    centroids.shrink_to_fit();
    
    for (auto& list : inverted_lists) {
        list.clear();
        list.shrink_to_fit();
    }
    
    kmeans.reset();
    dimensions = 0;
    is_trained = false;
    
    std::cout << "IVFFlat index cleared" << std::endl;
}

// Explicit template instantiations
template class IVFFlat<float>;
template class IVFFlat<double>;