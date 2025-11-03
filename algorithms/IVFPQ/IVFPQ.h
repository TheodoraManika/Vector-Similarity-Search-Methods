#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <utility>
#include <memory>
#include "../../metrics/metrics.h"
#include "../Kmeans/kmeans.h"

// Product Quantizer class
template <typename T>
class ProductQuantizer {
private:
    uint32_t M;           // Number of subspaces
    uint32_t codebook_size; // Size of each codebook (2^nbits)
    uint32_t subspace_dim;  // Dimension of each subspace
    std::vector<std::vector<std::vector<T>>> codebooks; // M codebooks

public:
    ProductQuantizer(uint32_t M, uint32_t codebook_size, uint32_t vector_dim);
    ~ProductQuantizer();
    
    void train(const std::vector<std::vector<double>>& training_data);
    std::vector<uint8_t> encode(const std::vector<double>& vector) const;
    std::vector<T> decode(const std::vector<uint8_t>& codes) const;
    std::vector<std::vector<T>> compute_distance_table(const std::vector<double>& query) const;
    
    uint32_t get_M() const { return M; }
    uint32_t get_codebook_size() const { return codebook_size; }
    uint32_t get_subspace_dim() const { return subspace_dim; }
};

// IVFPQ class
template <typename T>
class IVFPQ {
private:
    uint32_t n_clusters;     // Number of clusters
    uint32_t nprobe;         // Number of clusters to probe
    uint32_t M;              // Number of subspaces for PQ
    uint32_t nbits;          // Number of bits per subspace
    uint32_t vector_dim;     // Dimension of input vectors
    
    // Coarse quantizer (k-means for clustering)
    std::unique_ptr<KMeans<T>> coarse_quantizer;
    std::vector<std::vector<T>> centroids;
    
    // Product quantizer
    std::unique_ptr<ProductQuantizer<T>> product_quantizer;
    
    // Inverted lists storing encoded vectors
    std::vector<std::vector<std::pair<int, std::vector<uint8_t>>>> inverted_lists;
    
    // Original dataset for reference
    std::vector<std::vector<T>> dataset;

public:
    IVFPQ(uint32_t n_clusters, uint32_t nprobe, uint32_t M, uint32_t nbits, 
          uint32_t vector_dim, const std::vector<std::vector<T>>& data);
    ~IVFPQ();
    
    // Training and indexing
    void train();
    void add_vectors(const std::vector<std::vector<T>>& vectors);
    
    // Search functions
    std::pair<int, T> search_nearest(const std::vector<T>& query) const;
    std::list<std::pair<int, T>> search_k_nearest(const std::vector<T>& query, uint32_t k) const;
    std::list<std::pair<int, T>> range_search(const std::vector<T>& query, T radius) const;
    
private:
    // Helper functions
    std::vector<int> find_nearest_clusters(const std::vector<T>& query, uint32_t n_clusters_to_find) const;
    T compute_pq_distance(const std::vector<double>& query, const std::vector<uint8_t>& codes,
                         const std::vector<std::vector<T>>& distance_table) const;
};
