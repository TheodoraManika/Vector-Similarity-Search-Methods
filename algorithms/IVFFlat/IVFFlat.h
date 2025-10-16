#pragma once

#include <vector>
#include <memory>
#include <list>
#include <utility>
#include <stdexcept>
#include "../Kmeans/kmeans.h"
#include "../../metrics/metrics.h"
#include "../../utils/helpers/Utils.h"

template <typename T>
class IVFFlat {
private:
    uint32_t n_clusters;        // Number of clusters (kclusters)
    uint32_t n_probe;           // Number of clusters to probe during search
    uint32_t dimensions;        // Dimension of vectors
    bool is_trained;            // Flag to track if index is trained
    
    std::unique_ptr<KMeans<T>> kmeans;              // K-means clustering
    std::vector<std::vector<T>> centroids;          // Cluster centroids
    std::vector<std::vector<int>> inverted_lists;   // Inverted lists for each cluster
    std::vector<std::vector<T>> dataset;            // Original dataset
    
    // Helper methods
    std::vector<int> find_nearest_clusters(const std::vector<T>& query, uint32_t n_clusters) const;
    bool validate_query(const std::vector<T>& query) const;
    
public:
    IVFFlat(uint32_t n_clusters, uint32_t n_probe = 1);
    ~IVFFlat() = default;
    
    // Disable copy constructor and copy assignment (expensive operation)
    IVFFlat(const IVFFlat&) = delete;
    IVFFlat& operator=(const IVFFlat&) = delete;
    
    // Enable move constructor and move assignment
    IVFFlat(IVFFlat&&) noexcept = default;
    IVFFlat& operator=(IVFFlat&&) noexcept = default;
    
    // Train the index with dataset
    bool train(const std::vector<std::vector<T>>& data);
    
    // Add vectors to the index
    void add(const std::vector<std::vector<T>>& vectors);
    
    // Search methods
    std::pair<int, T> search_nearest(const std::vector<T>& query) const;
    std::list<std::pair<int, T>> search_k_nearest(const std::vector<T>& query, uint32_t k) const;
    std::list<std::pair<int, T>> range_search(const std::vector<T>& query, T radius) const;
    
    // Getters
    uint32_t get_n_clusters() const noexcept { return n_clusters; }
    uint32_t get_n_probe() const noexcept { return n_probe; }
    uint32_t get_dimensions() const noexcept { return dimensions; }
    size_t get_dataset_size() const noexcept { return dataset.size(); }
    bool trained() const noexcept { return is_trained; }
    const std::vector<std::vector<T>>& get_centroids() const { return centroids; }
    const std::vector<std::vector<T>>& get_dataset() const { return dataset; }
    
    // Setters
    void set_n_probe(uint32_t new_n_probe);
    
    // Utility methods
    void print_index_info() const;
    size_t get_memory_usage() const noexcept;
    void clear();
};