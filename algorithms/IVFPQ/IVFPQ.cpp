#include "IVFPQ.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <queue>
#include <unordered_set>

// ProductQuantizer implementation
template <typename T>
ProductQuantizer<T>::ProductQuantizer(uint32_t M, uint32_t codebook_size, uint32_t vector_dim)
    : M(M), codebook_size(codebook_size), subspace_dim(vector_dim / M) {
    if (vector_dim % M != 0) {
        throw std::invalid_argument("Vector dimension must be divisible by M");
    }
    codebooks.resize(M);
    for (uint32_t i = 0; i < M; ++i) {
        codebooks[i].resize(codebook_size);
        for (uint32_t j = 0; j < codebook_size; ++j) {
            codebooks[i][j].resize(subspace_dim);
        }
    }
}

template <typename T>
ProductQuantizer<T>::~ProductQuantizer() {}

template <typename T>
void ProductQuantizer<T>::train(const std::vector<std::vector<T>>& training_data) {
    // For each subspace, run k-means clustering
    for (uint32_t m = 0; m < M; ++m) {
        // Extract subspace data
        std::vector<std::vector<T>> subspace_data;
        subspace_data.reserve(training_data.size());
        
        for (const auto& vector : training_data) {
            std::vector<T> subvector(vector.begin() + m * subspace_dim,
                                   vector.begin() + (m + 1) * subspace_dim);
            subspace_data.push_back(subvector);
        }
        
        // Simple k-means for codebook generation 
        KMeans<T> kmeans(codebook_size, 100, static_cast<T>(1e-4));
        kmeans.set_data(subspace_data);
        kmeans.fit();
        
        // Get centroids as codebook
        auto clusters = kmeans.get_clusters();
        for (uint32_t k = 0; k < codebook_size && k < clusters.size(); ++k) {
            codebooks[m][k] = clusters[k].centroid;
        }
    }
}

template <typename T>
std::vector<uint8_t> ProductQuantizer<T>::encode(const std::vector<T>& vector) const {
    std::vector<uint8_t> codes(M);
    
    for (uint32_t m = 0; m < M; ++m) {
        T min_dist = std::numeric_limits<T>::max();
        uint8_t best_code = 0;
        
        // Extract subvector
        std::vector<T> subvector(vector.begin() + m * subspace_dim,
                               vector.begin() + (m + 1) * subspace_dim);
        
        // Find nearest centroid
        for (uint32_t k = 0; k < codebook_size; ++k) {
            T dist = metrics::EuclideanDistance(subvector, codebooks[m][k], subspace_dim);
            if (dist < min_dist) {
                min_dist = dist;
                best_code = static_cast<uint8_t>(k);
            }
        }
        
        codes[m] = best_code;
    }
    
    return codes;
}

template <typename T>
std::vector<T> ProductQuantizer<T>::decode(const std::vector<uint8_t>& codes) const {
    std::vector<T> vector;
    vector.reserve(M * subspace_dim);
    
    for (uint32_t m = 0; m < M; ++m) {
        const auto& centroid = codebooks[m][codes[m]];
        vector.insert(vector.end(), centroid.begin(), centroid.end());
    }
    
    return vector;
}

template <typename T>
std::vector<std::vector<T>> ProductQuantizer<T>::compute_distance_table(const std::vector<T>& query) const {
    std::vector<std::vector<T>> distance_table(M);
    
    for (uint32_t m = 0; m < M; ++m) {
        distance_table[m].resize(codebook_size);
        
        // Extract query subvector
        std::vector<T> query_subvec(query.begin() + m * subspace_dim,
                                  query.begin() + (m + 1) * subspace_dim);
        
        // Compute distances to all centroids in this subspace
        for (uint32_t centroid_id = 0; centroid_id < codebook_size; ++centroid_id) {
            distance_table[m][centroid_id] = 
                metrics::EuclideanDistance(query_subvec, codebooks[m][centroid_id], query_subvec.size());
        }
    }
    
    return distance_table;
}

// IVFPQ implementation
template <typename T>
IVFPQ<T>::IVFPQ(uint32_t n_clusters, uint32_t nprobe, uint32_t M, uint32_t nbits, 
                uint32_t vector_dim, const std::vector<std::vector<T>>& data)
    : n_clusters(n_clusters), nprobe(nprobe), M(M), nbits(nbits), 
      vector_dim(vector_dim), dataset(data) {
    
    uint32_t codebook_size = 1U << nbits; // 2^nbits
    
    // Initialize coarse quantizer
    coarse_quantizer = std::make_unique<KMeans<T>>(n_clusters, 100, static_cast<T>(1e-4));
    
    // Initialize product quantizer
    product_quantizer = std::make_unique<ProductQuantizer<T>>(M, codebook_size, vector_dim);
    
    // Initialize inverted lists
    inverted_lists.resize(n_clusters);
}

template <typename T>
IVFPQ<T>::~IVFPQ() {}

template <typename T>
void IVFPQ<T>::train() {
    // Train coarse quantizer
    coarse_quantizer->set_data(dataset);
    coarse_quantizer->fit();
    centroids = coarse_quantizer->get_centroids();
    
    // Train product quantizer
    product_quantizer->train(dataset);
}

template <typename T>
void IVFPQ<T>::add_vectors(const std::vector<std::vector<T>>& vectors) {
    for (size_t i = 0; i < vectors.size(); ++i) {
        // Assign to coarse cluster
        int cluster_id = coarse_quantizer->predict(vectors[i]);
        
        // Encode with product quantizer
        auto codes = product_quantizer->encode(vectors[i]);
        
        // Add to inverted list
        inverted_lists[cluster_id].emplace_back(static_cast<int>(i), codes);
    }
}

template <typename T>
std::pair<int, T> IVFPQ<T>::search_nearest(const std::vector<T>& query) const {
    auto results = search_k_nearest(query, 1);
    if (results.empty()) {
        return std::make_pair(-1, std::numeric_limits<T>::max());
    }
    return results.front();
}

template <typename T>
std::list<std::pair<int, T>> IVFPQ<T>::search_k_nearest(const std::vector<T>& query, uint32_t k) const {
    // Find nearest clusters to probe
    auto clusters = find_nearest_clusters(query, nprobe);
    
    // Compute distance table for query
    auto distance_table = product_quantizer->compute_distance_table(query);
    
    // Priority queue for k nearest neighbors
    std::priority_queue<std::pair<T, int>> pq;
    
    // Search in selected clusters
    for (int cluster_id : clusters) {
        const auto& inverted_list = inverted_lists[cluster_id];
        
        for (const auto& entry : inverted_list) {
            int vec_idx = entry.first;
            const auto& codes = entry.second;
            
            // Compute approximate distance using PQ
            T distance = compute_pq_distance(query, codes, distance_table);
            
            if (pq.size() < k) {
                pq.emplace(distance, vec_idx);
            } else if (distance < pq.top().first) {
                pq.pop();
                pq.emplace(distance, vec_idx);
            }
        }
    }
    
    // Convert to list
    std::list<std::pair<int, T>> result;
    while (!pq.empty()) {
        result.emplace_front(pq.top().second, pq.top().first);
        pq.pop();
    }
    
    return result;
}

template <typename T>
std::list<std::pair<int, T>> IVFPQ<T>::range_search(const std::vector<T>& query, T radius) const {
    // Find nearest clusters to probe
    auto clusters = find_nearest_clusters(query, nprobe);
    
    // Compute distance table for query
    auto distance_table = product_quantizer->compute_distance_table(query);
    
    std::list<std::pair<int, T>> result;
    
    // Search in selected clusters
    for (int cluster_id : clusters) {
        const auto& inverted_list = inverted_lists[cluster_id];
        
        for (const auto& entry : inverted_list) {
            int vec_idx = entry.first;
            const auto& codes = entry.second;
            
            // Compute approximate distance using PQ
            T distance = compute_pq_distance(query, codes, distance_table);
            
            if (distance <= radius) {
                result.emplace_back(vec_idx, distance);
            }
        }
    }
    
    return result;
}

template <typename T>
std::vector<int> IVFPQ<T>::find_nearest_clusters(const std::vector<T>& query, uint32_t n_clusters_to_find) const {
    std::vector<std::pair<T, int>> cluster_distances;
    cluster_distances.reserve(centroids.size());
    
    // Calculate distances to all centroids
    for (size_t i = 0; i < centroids.size(); ++i) {
        T distance = metrics::EuclideanDistance(query, centroids[i], query.size());
        cluster_distances.emplace_back(distance, static_cast<int>(i));
    }
    
    // Sort by distance and take n_clusters_to_find closest
    std::partial_sort(cluster_distances.begin(), 
                     cluster_distances.begin() + std::min(n_clusters_to_find, static_cast<uint32_t>(cluster_distances.size())),
                     cluster_distances.end());
    
    std::vector<int> result;
    result.reserve(n_clusters_to_find);
    for (uint32_t i = 0; i < std::min(n_clusters_to_find, static_cast<uint32_t>(cluster_distances.size())); ++i) {
        result.push_back(cluster_distances[i].second);
    }
    
    return result;
}

template <typename T>
T IVFPQ<T>::compute_pq_distance([[maybe_unused]] const std::vector<T>& query, const std::vector<uint8_t>& codes,
                                const std::vector<std::vector<T>>& distance_table) const {
    T total_distance = 0;
    
    for (uint32_t m = 0; m < M; ++m) {
        total_distance += distance_table[m][codes[m]];
    }
    
    return total_distance;
}

// Explicit template instantiation
template class ProductQuantizer<float>;
template class ProductQuantizer<double>;
template class IVFPQ<float>;
template class IVFPQ<double>;
