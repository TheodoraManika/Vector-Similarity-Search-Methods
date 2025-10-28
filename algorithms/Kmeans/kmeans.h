#pragma once

#include <vector>
#include <random>
#include <limits>
#include <algorithm>
#include <iostream>
#include <memory>
#include "../../metrics/metrics.h"
#include "../../utils/helpers/Utils.h"
#include "../LSH/LSH.h"  // Include LSH for approximate assignment

template <typename T>
struct Cluster {
    std::vector<T> centroid;
    std::vector<int> point_indices;
    
    Cluster(size_t dimensions) : centroid(dimensions, 0) {}
    
    void clear_points() {
        point_indices.clear();
    }
    
    void add_point(int index) {
        point_indices.push_back(index);
    }
    
    size_t size() const {
        return point_indices.size();
    }
};

template <typename T>
class KMeans {
private:
    uint32_t k;  // number of clusters
    uint32_t max_iterations;
    T tolerance;
    uint32_t dimensions;
    std::vector<std::vector<T>> data_points;
    std::vector<Cluster<T>> clusters;
    std::mt19937 rng;
    
    // LSH parameters for approximate assignment
    std::unique_ptr<LSH<T>> lsh_index;
    bool use_lsh;
    uint32_t lsh_k;      // Number of hash functions per table
    uint32_t lsh_L;      // Number of hash tables
    T lsh_w;             // Cell width
    T lsh_radius;        // Radius for range search
    
    // Helper methods
    void initialize_centroids_random();
    void initialize_centroids_kmeans_plus_plus();
    void assign_points_to_clusters();
    void assign_points_to_clusters_lsh();           // LSH-based assignment
    void assign_points_to_clusters_reverse_lsh();   // Reverse assignment with LSH
    bool update_centroids();
    T compute_within_cluster_sum_of_squares() const;
    T compute_silhouette_score() const;
    void build_lsh_index();                         // Build LSH index on data points
    
public:
    // Constructor with optional LSH parameters
    KMeans(uint32_t k, uint32_t max_iterations = 100, T tolerance = 1e-4);
    
    // Constructor with LSH enabled
    KMeans(uint32_t k, uint32_t max_iterations, T tolerance,
           uint32_t lsh_k, uint32_t lsh_L, T lsh_w, T lsh_radius);
    
    // Set data points for clustering
    void set_data(const std::vector<std::vector<T>>& data);
    
    // Enable/disable LSH for assignment
    void enable_lsh(bool enable = true);
    void set_lsh_parameters(uint32_t k, uint32_t L, T w, T radius);
    
    // Perform clustering
    bool fit();
    bool fit_with_lsh();           // Clustering with LSH-based assignment
    bool fit_reverse_lsh();        // Clustering with reverse LSH assignment
    
    // Predict cluster for new points
    int predict(const std::vector<T>& point) const;
    std::vector<int> predict(const std::vector<std::vector<T>>& points) const;
    
    // Get results
    const std::vector<Cluster<T>>& get_clusters() const { return clusters; }
    std::vector<std::vector<T>> get_centroids() const;
    std::vector<int> get_labels() const;
    
    // Evaluation metrics
    T get_inertia() const;
    T get_silhouette_score() const;                          // Overall clustering silhouette
    std::vector<T> get_cluster_silhouettes() const;          // Per-cluster average silhouettes
    std::vector<T> get_point_silhouettes() const;            // Individual point silhouettes
    void print_silhouette_analysis() const;                  // Detailed silhouette report
    
    // Utility methods
    void print_cluster_info() const;
    static uint32_t find_optimal_k(const std::vector<std::vector<T>>& data, uint32_t min_k, uint32_t max_k);
};