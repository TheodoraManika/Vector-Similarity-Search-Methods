#include "kmeans.h"
#include <cmath>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <iomanip>

template <typename T>
KMeans<T>::KMeans(uint32_t k, uint32_t max_iterations, T tolerance)
    : k(k), max_iterations(max_iterations), tolerance(tolerance), dimensions(0),
      use_lsh(false), lsh_k(4), lsh_L(5), lsh_w(4.0), lsh_radius(1000.0) {
    std::random_device rd;
    rng.seed(rd());
}

template <typename T>
KMeans<T>::KMeans(uint32_t k, uint32_t max_iterations, T tolerance,
                  uint32_t lsh_k, uint32_t lsh_L, T lsh_w, T lsh_radius)
    : k(k), max_iterations(max_iterations), tolerance(tolerance), dimensions(0),
      use_lsh(true), lsh_k(lsh_k), lsh_L(lsh_L), lsh_w(lsh_w), lsh_radius(lsh_radius) {
    std::random_device rd;
    rng.seed(rd());
}

template <typename T>
void KMeans<T>::set_data(const std::vector<std::vector<T>>& data) {
    if (data.empty()) {
        throw std::invalid_argument("Data cannot be empty");
    }
    
    data_points = data;
    dimensions = data[0].size();
    
    // Validate dimensions
    for (const auto& point : data_points) {
        if (point.size() != dimensions) {
            throw std::invalid_argument("All data points must have same dimensions");
        }
    }
    
    // Initialize clusters
    clusters.clear();
    clusters.reserve(k);
    for (uint32_t i = 0; i < k; ++i) {
        clusters.emplace_back(dimensions);
    }
}

template <typename T>
void KMeans<T>::enable_lsh(bool enable) {
    use_lsh = enable;
}

template <typename T>
void KMeans<T>::set_lsh_parameters(uint32_t k, uint32_t L, T w, T radius) {
    lsh_k = k;
    lsh_L = L;
    lsh_w = w;
    lsh_radius = radius;
}

template <typename T>
void KMeans<T>::build_lsh_index() {
    if (data_points.empty()) {
        throw std::runtime_error("Cannot build LSH index: no data points");
    }
    
    // Calculate M (hash table size) as a function of data size
    uint32_t M = static_cast<uint32_t>(data_points.size() / lsh_L) + 1;
    
    // Create LSH index with data points
    lsh_index = std::make_unique<LSH<T>>(
        lsh_L,                    // L: number of hash tables
        4294967291,               // m: large prime (2^32 - 5)
        M,                        // M: hash table size
        data_points.size(),       // n_points: number of points
        lsh_k,                    // k: hash functions per table
        dimensions,               // space_dim: dimensionality
        static_cast<uint32_t>(lsh_w),  // w: cell width
        data_points               // init_vectors: the data
    );
    
    std::cout << "Built LSH index with L=" << lsh_L << ", k=" << lsh_k 
              << ", w=" << lsh_w << ", M=" << M << std::endl;
}

template <typename T>
void KMeans<T>::initialize_centroids_random() {
    std::uniform_int_distribution<> dis(0, data_points.size() - 1);
    std::unordered_set<int> selected_indices;
    
    for (uint32_t i = 0; i < k; ++i) {
        int index;
        do {
            index = dis(rng);
        } while (selected_indices.find(index) != selected_indices.end());
        
        selected_indices.insert(index);
        clusters[i].centroid = data_points[index];
    }
}

template <typename T>
void KMeans<T>::initialize_centroids_kmeans_plus_plus() {
    // k-means++ initialization [Arthur-Vassilvitskii:SODA'07]
    // Expected approximation ratio: O(log k)
    // Algorithm:
    // (1) Choose a centroid uniformly at random; t ← 1
    // (2) For all non-centroid points i: D(i) ← min distance to existing centroids
    // (3) Choose new centroid r with probability ∝ D(r)²:
    //     prob[choose r] = D(r)² / Σ D(i)²
    // (4) Let t ← t + 1, repeat (2)-(3) until t = k
    
    // Efficient implementation using partial sums and binary search:
    // Compute partial sums P(r) = Σ(i=1 to r) D(i)²
    // Normalize D(i) by max D(i) to avoid overflow
    // Use binary search to find r where P(r-1) < x ≤ P(r)
    
    // Step (1): Choose first centroid uniformly at random
    std::uniform_int_distribution<> dis(0, data_points.size() - 1);
    clusters[0].centroid = data_points[dis(rng)];
    
    // Steps (2)-(4): Choose remaining centroids using k-means++
    for (uint32_t cluster_idx = 1; cluster_idx < k; ++cluster_idx) {  // t = 1 to k-1
        std::vector<T> distances(data_points.size());
        T max_distance = 0;
        
        // Step (2): Compute D(i) = min distance to existing centroids
        for (size_t point_idx = 0; point_idx < data_points.size(); ++point_idx) {
            T min_distance = std::numeric_limits<T>::max();
            
            // D(i) = min distance to any of the t chosen centroids
            for (uint32_t existing_cluster = 0; existing_cluster < cluster_idx; ++existing_cluster) {
                T distance = metrics::EuclideanDistance(data_points[point_idx], 
                                                       clusters[existing_cluster].centroid,
                                                       data_points[point_idx].size());
                min_distance = std::min(min_distance, distance);
            }
            
            distances[point_idx] = min_distance;
            max_distance = std::max(max_distance, min_distance);
        }
        
        // Normalize distances by max D(i) to avoid overflow
        // and compute partial sums P(r) = Σ(i=1 to r) D(i)²
        std::vector<T> partial_sums(data_points.size());
        T cumulative = 0;
        
        for (size_t point_idx = 0; point_idx < data_points.size(); ++point_idx) {
            // Normalize: D(i) / max D(i), then square
            T normalized_dist = (max_distance > 0) ? (distances[point_idx] / max_distance) : 0;
            T squared_dist = normalized_dist * normalized_dist;
            
            // P(r) = Σ(i=1 to r) D(i)²
            cumulative += squared_dist;
            partial_sums[point_idx] = cumulative;
        }
        
        // Step (3): Choose next centroid with probability ∝ D(r)²
        // Pick uniformly distributed x ∈ [0, P(n-t)]
        T total_sum = partial_sums.back();
        if (total_sum <= 0) {
            // Fallback to random selection if all distances are 0
            clusters[cluster_idx].centroid = data_points[dis(rng)];
            continue;
        }
        
        std::uniform_real_distribution<T> prob_dis(0, total_sum);
        T target = prob_dis(rng);
        
        // Binary search to find r where P(r-1) < target ≤ P(r)
        // This is equivalent to std::lower_bound
        auto it = std::lower_bound(partial_sums.begin(), partial_sums.end(), target);
        size_t selected_idx = std::distance(partial_sums.begin(), it);
        
        // Ensure valid index
        if (selected_idx >= data_points.size()) {
            selected_idx = data_points.size() - 1;
        }
        
        clusters[cluster_idx].centroid = data_points[selected_idx];
    }
}

template <typename T>
void KMeans<T>::assign_points_to_clusters() {
    // E-step (Expectation): Assignment phase
    // For each point xi, find d(xi, C) = min_{c∈C} d(xi, c)
    // Assign xi to cluster with nearest centroid
    // Complexity: O(nkd) where n=#points, k=#clusters, d=#dimensions
    
    // Clear existing assignments
    for (auto& cluster : clusters) {
        cluster.clear_points();
    }
    
    // Assign each point to nearest cluster
    for (size_t point_idx = 0; point_idx < data_points.size(); ++point_idx) {
        T min_distance = std::numeric_limits<T>::max();
        uint32_t best_cluster = 0;
        
        // Find nearest centroid: min_{c∈C} d(xi, c)
        for (uint32_t cluster_idx = 0; cluster_idx < k; ++cluster_idx) {
            T distance = metrics::EuclideanDistance(data_points[point_idx], 
                                                   clusters[cluster_idx].centroid,
                                                   data_points[point_idx].size());
            if (distance < min_distance) {
                min_distance = distance;
                best_cluster = cluster_idx;
            }
        }
        
        clusters[best_cluster].add_point(static_cast<int>(point_idx));
    }
}

template <typename T>
void KMeans<T>::assign_points_to_clusters_lsh() {
    // Clear existing assignments
    for (auto& cluster : clusters) {
        cluster.clear_points();
    }
    
    // For each point, use LSH to find candidate nearby centroids quickly
    for (size_t point_idx = 0; point_idx < data_points.size(); ++point_idx) {
        T min_distance = std::numeric_limits<T>::max();
        uint32_t best_cluster = 0;
        
        // Use LSH to get approximate nearest neighbors (centroids)
        // We search among centroids, not data points
        std::vector<int> candidate_clusters;
        
        // Get k-nearest centroids using LSH approximate search
        // Note: This requires querying the centroid space, not the data space
        // For now, we'll use a hybrid approach: LSH narrows down candidates
        
        // Use LSH to find points near this data point, then check their cluster assignments
        // This is an approximation - we're finding similar points and checking their clusters
        auto nn_result = lsh_index->NearestNeighbour(data_points[point_idx]);
        int nearest_point_idx = nn_result.first;
        
        if (nearest_point_idx >= 0) {
            // Find which cluster this nearest neighbor belongs to
            for (size_t cluster_idx = 0; cluster_idx < clusters.size(); ++cluster_idx) {
                const auto& indices = clusters[cluster_idx].point_indices;
                if (std::find(indices.begin(), indices.end(), nearest_point_idx) != indices.end()) {
                    // Start with this cluster as primary candidate
                    T distance = metrics::EuclideanDistance(data_points[point_idx], 
                                                           clusters[cluster_idx].centroid,
                                                           data_points[point_idx].size());
                    if (distance < min_distance) {
                        min_distance = distance;
                        best_cluster = cluster_idx;
                    }
                    break;
                }
            }
        }
        
        // Also check a few nearby clusters to improve accuracy
        for (uint32_t cluster_idx = 0; cluster_idx < k; ++cluster_idx) {
            T distance = metrics::EuclideanDistance(data_points[point_idx], 
                                                   clusters[cluster_idx].centroid,
                                                   data_points[point_idx].size());
            if (distance < min_distance) {
                min_distance = distance;
                best_cluster = cluster_idx;
            }
        }
        
        clusters[best_cluster].add_point(static_cast<int>(point_idx));
    }
}

template <typename T>
void KMeans<T>::assign_points_to_clusters_reverse_lsh() {
    // Clear existing assignments
    for (auto& cluster : clusters) {
        cluster.clear_points();
    }
    
    // Track which points have been assigned
    std::vector<bool> assigned(data_points.size(), false);
    
    // Reverse assignment: For each centroid, use LSH range search to find nearby points
    for (uint32_t cluster_idx = 0; cluster_idx < k; ++cluster_idx) {
        const auto& centroid = clusters[cluster_idx].centroid;
        
        // Use LSH range search to find points within radius of this centroid
        // RangeSearch returns list<pair<int, T>> where int is index and T is distance
        auto nearby_points_with_dist = lsh_index->RangeSearch(centroid, lsh_radius, 1);
        
        // Assign each nearby point to this cluster if it's the closest centroid
        for (const auto& point_pair : nearby_points_with_dist) {
            int point_idx = point_pair.first;
            
            if (point_idx < 0 || point_idx >= static_cast<int>(data_points.size())) {
                continue;
            }
            
            // Check if this centroid is actually the closest
            T distance_to_this = metrics::EuclideanDistance(data_points[point_idx], 
                                                           centroid,
                                                           data_points[point_idx].size());
            
            // Check if any other centroid is closer
            bool is_closest = true;
            for (uint32_t other_idx = 0; other_idx < k; ++other_idx) {
                if (other_idx == cluster_idx) continue;
                
                T distance_to_other = metrics::EuclideanDistance(data_points[point_idx],
                                                                clusters[other_idx].centroid,
                                                                data_points[point_idx].size());
                if (distance_to_other < distance_to_this) {
                    is_closest = false;
                    break;
                }
            }
            
            if (is_closest && !assigned[point_idx]) {
                clusters[cluster_idx].add_point(point_idx);
                assigned[point_idx] = true;
            }
        }
    }
    
    // Assign any remaining unassigned points using brute force
    for (size_t point_idx = 0; point_idx < data_points.size(); ++point_idx) {
        if (!assigned[point_idx]) {
            T min_distance = std::numeric_limits<T>::max();
            uint32_t best_cluster = 0;
            
            for (uint32_t cluster_idx = 0; cluster_idx < k; ++cluster_idx) {
                T distance = metrics::EuclideanDistance(data_points[point_idx],
                                                       clusters[cluster_idx].centroid,
                                                       data_points[point_idx].size());
                if (distance < min_distance) {
                    min_distance = distance;
                    best_cluster = cluster_idx;
                }
            }
            
            clusters[best_cluster].add_point(static_cast<int>(point_idx));
            assigned[point_idx] = true;
        }
    }
}

template <typename T>
bool KMeans<T>::update_centroids() {
    // M-step (Maximization): Update phase
    // For each cluster C, calculate new centroid as mean of assigned points:
    // centroid = (1/T) * Σ vi where T = #points in cluster
    // Complexity: O(nd) where n=#points, d=#dimensions
    
    bool converged = true;
    
    for (uint32_t cluster_idx = 0; cluster_idx < k; ++cluster_idx) {
        auto& cluster = clusters[cluster_idx];
        
        if (cluster.size() == 0) {
            // Handle empty cluster by reinitializing
            std::uniform_int_distribution<> dis(0, data_points.size() - 1);
            cluster.centroid = data_points[dis(rng)];
            converged = false;
            continue;
        }
        
        // Calculate new centroid: (1/T) * Σ vi
        std::vector<T> new_centroid(dimensions, 0);
        
        // Sum all points in cluster: Σ vi
        for (int point_idx : cluster.point_indices) {
            for (size_t dim = 0; dim < dimensions; ++dim) {
                new_centroid[dim] += data_points[point_idx][dim];
            }
        }
        
        // Divide by T (number of points in cluster): (1/T) * Σ vi
        for (size_t dim = 0; dim < dimensions; ++dim) {
            new_centroid[dim] /= cluster.size();
        }
        
        // Check for convergence (little change in centroids)
        T distance = metrics::EuclideanDistance(cluster.centroid, new_centroid, cluster.centroid.size());
        if (distance > tolerance) {
            converged = false;
        }
        
        cluster.centroid = std::move(new_centroid);
    }
    
    return converged;
}

template <typename T>
bool KMeans<T>::fit() {
    if (data_points.empty() || k == 0) {
        return false;
    }
    
    if (k > data_points.size()) {
        std::cerr << "Warning: k (" << k << ") is larger than number of data points (" 
                  << data_points.size() << ")" << std::endl;
        k = data_points.size();
    }
    
    // Initialize centroids using k-means++
    initialize_centroids_kmeans_plus_plus();
    
    // Track objective value (inertia) for convergence monitoring
    T previous_inertia = std::numeric_limits<T>::max();
    
    // Main k-means loop
    for (uint32_t iteration = 0; iteration < max_iterations; ++iteration) {
        // E-step: Assignment - assign each point to nearest centroid
        assign_points_to_clusters();
        
        // M-step: Update - recalculate centroids as mean of assigned points
        bool centroids_converged = update_centroids();
        
        // Calculate k-means objective: Σ d(xi, C)²
        T current_inertia = get_inertia();
        
        // Check convergence based on objective change
        T inertia_change = std::abs(previous_inertia - current_inertia);
        
        if (centroids_converged && inertia_change < tolerance) {
            std::cout << "K-means converged after " << iteration + 1 << " iterations" << std::endl;
            std::cout << "Final objective (inertia): " << current_inertia << std::endl;
            return true;
        }
        
        previous_inertia = current_inertia;
    }
    
    std::cout << "K-means reached maximum iterations (" << max_iterations << ")" << std::endl;
    std::cout << "Final objective (inertia): " << previous_inertia << std::endl;
    return false;
}

template <typename T>
bool KMeans<T>::fit_with_lsh() {
    if (data_points.empty() || k == 0) {
        return false;
    }
    
    if (k > data_points.size()) {
        std::cerr << "Warning: k (" << k << ") is larger than number of data points (" 
                  << data_points.size() << ")" << std::endl;
        k = data_points.size();
    }
    
    // Initialize centroids using k-means++
    initialize_centroids_kmeans_plus_plus();
    
    // Build LSH index on data points (one time)
    std::cout << "Building LSH index for K-means clustering..." << std::endl;
    build_lsh_index();
    
    // Main k-means loop with LSH-based assignment
    for (uint32_t iteration = 0; iteration < max_iterations; ++iteration) {
        assign_points_to_clusters_lsh();
        
        if (update_centroids()) {
            std::cout << "K-means (LSH) converged after " << iteration + 1 << " iterations" << std::endl;
            return true;
        }
        
        // Rebuild LSH index periodically for better accuracy (every 5 iterations)
        if ((iteration + 1) % 5 == 0) {
            std::cout << "Rebuilding LSH index at iteration " << iteration + 1 << std::endl;
            build_lsh_index();
        }
    }
    
    std::cout << "K-means (LSH) reached maximum iterations (" << max_iterations << ")" << std::endl;
    return false;
}

template <typename T>
bool KMeans<T>::fit_reverse_lsh() {
    if (data_points.empty() || k == 0) {
        return false;
    }
    
    if (k > data_points.size()) {
        std::cerr << "Warning: k (" << k << ") is larger than number of data points (" 
                  << data_points.size() << ")" << std::endl;
        k = data_points.size();
    }
    
    // Initialize centroids using k-means++
    initialize_centroids_kmeans_plus_plus();
    
    // Build LSH index on data points (one time)
    std::cout << "Building LSH index for reverse assignment K-means..." << std::endl;
    build_lsh_index();
    
    // Main k-means loop with reverse LSH-based assignment
    for (uint32_t iteration = 0; iteration < max_iterations; ++iteration) {
        assign_points_to_clusters_reverse_lsh();
        
        if (update_centroids()) {
            std::cout << "K-means (Reverse LSH) converged after " << iteration + 1 << " iterations" << std::endl;
            return true;
        }
        
        // Rebuild LSH index periodically (every 5 iterations)
        if ((iteration + 1) % 5 == 0) {
            std::cout << "Rebuilding LSH index at iteration " << iteration + 1 << std::endl;
            build_lsh_index();
        }
    }
    
    std::cout << "K-means (Reverse LSH) reached maximum iterations (" << max_iterations << ")" << std::endl;
    return false;
}

template <typename T>
int KMeans<T>::predict(const std::vector<T>& point) const {
    T min_distance = std::numeric_limits<T>::max();
    int best_cluster = -1;
    
    for (size_t cluster_idx = 0; cluster_idx < clusters.size(); ++cluster_idx) {
        T distance = metrics::EuclideanDistance(point, clusters[cluster_idx].centroid, point.size());
        if (distance < min_distance) {
            min_distance = distance;
            best_cluster = static_cast<int>(cluster_idx);
        }
    }
    
    return best_cluster;
}

template <typename T>
std::vector<int> KMeans<T>::predict(const std::vector<std::vector<T>>& points) const {
    std::vector<int> labels;
    labels.reserve(points.size());
    
    for (const auto& point : points) {
        labels.push_back(predict(point));
    }
    
    return labels;
}

template <typename T>
std::vector<std::vector<T>> KMeans<T>::get_centroids() const {
    std::vector<std::vector<T>> centroids;
    centroids.reserve(clusters.size());
    
    for (const auto& cluster : clusters) {
        centroids.push_back(cluster.centroid);
    }
    
    return centroids;
}

template <typename T>
std::vector<int> KMeans<T>::get_labels() const {
    std::vector<int> labels(data_points.size(), -1);
    
    for (size_t cluster_idx = 0; cluster_idx < clusters.size(); ++cluster_idx) {
        for (int point_idx : clusters[cluster_idx].point_indices) {
            labels[point_idx] = static_cast<int>(cluster_idx);
        }
    }
    
    return labels;
}

template <typename T>
T KMeans<T>::get_inertia() const {
    // K-means objective function: min_{C⊆Z,|C|=k} Σ d(xi, C)²
    // where d(xi, C) = min_{c∈C} d(xi, c)
    // This is the sum of squared distances from each point to its assigned centroid
    // Also known as "within-cluster sum of squares" (WCSS) or "inertia"
    T inertia = 0;
    
    for (const auto& cluster : clusters) {
        for (int point_idx : cluster.point_indices) {
            // d(xi, centroid) using Euclidean distance (ℓ2 metric)
            T distance = metrics::EuclideanDistance(data_points[point_idx], cluster.centroid, data_points[point_idx].size());
            // Sum of squared distances: Σ d(xi, C)²
            inertia += distance * distance;
        }
    }
    
    return inertia;
}

template <typename T>
T KMeans<T>::get_silhouette_score() const {
    // Silhouette Coefficient: measures quality of clustering
    // Average silhouette over all points ∈ [-1, 1]
    // s(i) → 1: point i well-clustered (correctly assigned)
    // s(i) ≃ 0: borderline assignment (on cluster boundary)
    // s(i) → -1: point i would be better in next best cluster (misclassified)
    
    if (clusters.size() < 2) return 0;  // Need at least 2 clusters
    
    std::vector<int> labels = get_labels();
    T total_silhouette = 0;
    int valid_points = 0;
    
    for (size_t i = 0; i < data_points.size(); ++i) {
        int cluster_a = labels[i];
        if (clusters[cluster_a].size() <= 1) continue;  // Skip singleton clusters
        
        // a(i) = average distance of point i to all other points in same cluster
        // a(i) measures how well point i fits into its assigned cluster
        T a = 0;
        for (int j : clusters[cluster_a].point_indices) {
            if (j != static_cast<int>(i)) {
                a += metrics::EuclideanDistance(data_points[i], data_points[j], data_points[i].size());
            }
        }
        a /= (clusters[cluster_a].size() - 1);  // Average over all other points in cluster
        
        // b(i) = average distance of point i to all points in next best (neighbor) cluster
        // "Next best" = cluster with minimum average distance (excluding current cluster)
        // This is the cluster of the 2nd closest centroid
        T b = std::numeric_limits<T>::max();
        for (size_t cluster_idx = 0; cluster_idx < clusters.size(); ++cluster_idx) {
            if (cluster_idx == static_cast<size_t>(cluster_a) || clusters[cluster_idx].size() == 0) {
                continue;  // Skip current cluster and empty clusters
            }
            
            T avg_distance = 0;
            for (int j : clusters[cluster_idx].point_indices) {
                avg_distance += metrics::EuclideanDistance(data_points[i], data_points[j], data_points[i].size());
            }
            avg_distance /= clusters[cluster_idx].size();
            b = std::min(b, avg_distance);  // Find minimum average distance
        }
        
        // Silhouette coefficient for point i:
        // s(i) = (b(i) - a(i)) / max{a(i), b(i)} ∈ [-1, 1]
        //
        // Equivalent formulation:
        //   s(i) = 1 - a(i)/b(i),     if a(i) < b(i)  [good: closer to own cluster]
        //   s(i) = 0,                  if a(i) = b(i)  [borderline]
        //   s(i) = b(i)/a(i) - 1,     if a(i) > b(i)  [bad: closer to other cluster]
        if (b != std::numeric_limits<T>::max()) {
            T silhouette = (b - a) / std::max(a, b);
            total_silhouette += silhouette;
            valid_points++;
        }
    }
    
    // Return average silhouette coefficient over all valid points
    return valid_points > 0 ? total_silhouette / valid_points : 0;
}

template <typename T>
std::vector<T> KMeans<T>::get_point_silhouettes() const {
    // Compute silhouette coefficient for each individual point
    // Returns vector of s(i) values for all points i = 0, ..., n-1
    // Points in singleton clusters get s(i) = 0
    
    std::vector<T> silhouettes(data_points.size(), 0);
    
    if (clusters.size() < 2) {
        return silhouettes;  // Need at least 2 clusters
    }
    
    std::vector<int> labels = get_labels();
    
    for (size_t i = 0; i < data_points.size(); ++i) {
        int cluster_a = labels[i];
        
        // Singleton clusters have undefined silhouette, set to 0
        if (clusters[cluster_a].size() <= 1) {
            silhouettes[i] = 0;
            continue;
        }
        
        // Calculate a(i) - average distance to points in same cluster
        T a = 0;
        for (int j : clusters[cluster_a].point_indices) {
            if (j != static_cast<int>(i)) {
                a += metrics::EuclideanDistance(data_points[i], data_points[j], data_points[i].size());
            }
        }
        a /= (clusters[cluster_a].size() - 1);
        
        // Calculate b(i) - minimum average distance to points in other clusters
        T b = std::numeric_limits<T>::max();
        for (size_t cluster_idx = 0; cluster_idx < clusters.size(); ++cluster_idx) {
            if (cluster_idx == static_cast<size_t>(cluster_a) || clusters[cluster_idx].size() == 0) {
                continue;
            }
            
            T avg_distance = 0;
            for (int j : clusters[cluster_idx].point_indices) {
                avg_distance += metrics::EuclideanDistance(data_points[i], data_points[j], data_points[i].size());
            }
            avg_distance /= clusters[cluster_idx].size();
            b = std::min(b, avg_distance);
        }
        
        // Calculate silhouette coefficient
        if (b != std::numeric_limits<T>::max()) {
            silhouettes[i] = (b - a) / std::max(a, b);
        }
    }
    
    return silhouettes;
}

template <typename T>
std::vector<T> KMeans<T>::get_cluster_silhouettes() const {
    // Compute average silhouette coefficient for each cluster
    // Returns vector of average s(i) for points in each cluster
    // 
    // Interpretation:
    // - High value: cluster is well-separated and cohesive
    // - Low value: cluster may indicate poor k choice or outliers
    // 
    // If k is too large/small, some clusters will have much smaller
    // silhouettes than others
    
    std::vector<T> cluster_silhouettes(clusters.size(), 0);
    
    if (clusters.size() < 2) {
        return cluster_silhouettes;  // Need at least 2 clusters
    }
    
    // Get silhouette for each point
    std::vector<T> point_silhouettes = get_point_silhouettes();
    std::vector<int> labels = get_labels();
    
    // Accumulate silhouettes per cluster
    std::vector<T> cluster_sums(clusters.size(), 0);
    std::vector<int> cluster_counts(clusters.size(), 0);
    
    for (size_t i = 0; i < data_points.size(); ++i) {
        int cluster_idx = labels[i];
        
        // Only count non-singleton clusters
        if (clusters[cluster_idx].size() > 1) {
            cluster_sums[cluster_idx] += point_silhouettes[i];
            cluster_counts[cluster_idx]++;
        }
    }
    
    // Compute average per cluster
    for (size_t c = 0; c < clusters.size(); ++c) {
        if (cluster_counts[c] > 0) {
            cluster_silhouettes[c] = cluster_sums[c] / cluster_counts[c];
        }
    }
    
    return cluster_silhouettes;
}

template <typename T>
void KMeans<T>::print_silhouette_analysis() const {
    // Print detailed silhouette analysis for clustering quality assessment
    // Includes overall score, per-cluster scores, and interpretation
    
    std::cout << "\n=== Silhouette Analysis ===" << std::endl;
    std::cout << std::fixed << std::setprecision(4);
    
    // Overall silhouette
    T overall_silhouette = get_silhouette_score();
    std::cout << "\nOverall Silhouette Coefficient: " << overall_silhouette << std::endl;
    std::cout << "Interpretation: ";
    if (overall_silhouette > 0.70) {
        std::cout << "Excellent clustering (strong structure)" << std::endl;
    } else if (overall_silhouette > 0.50) {
        std::cout << "Good clustering (reasonable structure)" << std::endl;
    } else if (overall_silhouette > 0.25) {
        std::cout << "Fair clustering (weak structure)" << std::endl;
    } else {
        std::cout << "Poor clustering (no substantial structure)" << std::endl;
    }
    
    // Per-cluster silhouettes
    std::vector<T> cluster_silhouettes = get_cluster_silhouettes();
    
    std::cout << "\nPer-Cluster Silhouette Scores:" << std::endl;
    std::cout << "-------------------------------" << std::endl;
    
    T min_silhouette = std::numeric_limits<T>::max();
    T max_silhouette = std::numeric_limits<T>::lowest();
    size_t min_cluster = 0, max_cluster = 0;
    
    for (size_t c = 0; c < cluster_silhouettes.size(); ++c) {
        if (clusters[c].size() == 0) continue;
        
        T score = cluster_silhouettes[c];
        std::cout << "  Cluster " << c << " (n=" << std::setw(5) << clusters[c].size() 
                  << "): " << std::setw(7) << score;
        
        // Track min/max
        if (score < min_silhouette && clusters[c].size() > 1) {
            min_silhouette = score;
            min_cluster = c;
        }
        if (score > max_silhouette) {
            max_silhouette = score;
            max_cluster = c;
        }
        
        // Add interpretation
        if (score > 0.70) {
            std::cout << " [Excellent]";
        } else if (score > 0.50) {
            std::cout << " [Good]";
        } else if (score > 0.25) {
            std::cout << " [Fair]";
        } else if (score > 0) {
            std::cout << " [Poor]";
        } else if (clusters[c].size() == 1) {
            std::cout << " [Singleton]";
        }
        std::cout << std::endl;
    }
    
    // Analysis of cluster quality variation
    if (cluster_silhouettes.size() > 1 && max_silhouette > min_silhouette) {
        T silhouette_range = max_silhouette - min_silhouette;
        
        std::cout << "\nCluster Quality Analysis:" << std::endl;
        std::cout << "  Best cluster:  " << max_cluster << " (s=" << max_silhouette << ")" << std::endl;
        std::cout << "  Worst cluster: " << min_cluster << " (s=" << min_silhouette << ")" << std::endl;
        std::cout << "  Range: " << silhouette_range << std::endl;
        
        if (silhouette_range > 0.3) {
            std::cout << "\n⚠ WARNING: Large variation in cluster quality!" << std::endl;
            std::cout << "  This may indicate:" << std::endl;
            std::cout << "  - k is too large (over-clustering)" << std::endl;
            std::cout << "  - k is too small (under-clustering)" << std::endl;
            std::cout << "  - Presence of outliers" << std::endl;
            std::cout << "  - Unbalanced cluster sizes" << std::endl;
            std::cout << "\n  Suggestion: Try different values of k and compare" << std::endl;
            std::cout << "              silhouette plots for more uniform scores." << std::endl;
        } else {
            std::cout << "\n✓ Clusters have roughly equal silhouettes (good k choice)" << std::endl;
        }
    }
    
    // Distribution statistics
    std::vector<T> point_silhouettes = get_point_silhouettes();
    int well_clustered = 0, borderline = 0, misclassified = 0;
    
    for (T s : point_silhouettes) {
        if (s > 0.5) well_clustered++;
        else if (s > -0.1) borderline++;
        else misclassified++;
    }
    
    std::cout << "\nPoint Classification:" << std::endl;
    std::cout << "  Well clustered (s > 0.5):   " << std::setw(5) << well_clustered 
              << " (" << std::setw(5) << std::setprecision(1) 
              << (100.0 * well_clustered / data_points.size()) << "%)" << std::endl;
    std::cout << "  Borderline (-0.1 < s ≤ 0.5): " << std::setw(5) << borderline 
              << " (" << std::setw(5) << std::setprecision(1) 
              << (100.0 * borderline / data_points.size()) << "%)" << std::endl;
    std::cout << "  Misclassified (s ≤ -0.1):   " << std::setw(5) << misclassified 
              << " (" << std::setw(5) << std::setprecision(1) 
              << (100.0 * misclassified / data_points.size()) << "%)" << std::endl;
    
    std::cout << "\n==========================" << std::endl;
}

template <typename T>
void KMeans<T>::print_cluster_info() const {
    std::cout << "=== K-means Clustering Results ===" << std::endl;
    std::cout << "Number of clusters: " << clusters.size() << std::endl;
    std::cout << "Number of data points: " << data_points.size() << std::endl;
    
    for (size_t i = 0; i < clusters.size(); ++i) {
        std::cout << "Cluster " << i << ": " << clusters[i].size() << " points" << std::endl;
    }
    
    std::cout << "Inertia: " << get_inertia() << std::endl;
    std::cout << "Silhouette score: " << get_silhouette_score() << std::endl;
    std::cout << "=================================" << std::endl;
}

template <typename T>
uint32_t KMeans<T>::find_optimal_k(const std::vector<std::vector<T>>& data, uint32_t min_k, uint32_t max_k) {
    std::vector<T> silhouette_scores;
    uint32_t best_k = min_k;
    T best_score = -1;
    
    for (uint32_t k = min_k; k <= max_k; ++k) {
        KMeans<T> kmeans(k);
        kmeans.set_data(data);
        kmeans.fit();
        
        T score = kmeans.get_silhouette_score();
        silhouette_scores.push_back(score);
        
        if (score > best_score) {
            best_score = score;
            best_k = k;
        }
        
        std::cout << "k=" << k << ", silhouette score=" << score << std::endl;
    }
    
    std::cout << "Optimal k: " << best_k << " (silhouette score: " << best_score << ")" << std::endl;
    return best_k;
}

// Explicit template instantiations
template class KMeans<float>;
template class KMeans<double>;