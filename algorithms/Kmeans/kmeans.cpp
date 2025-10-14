#include "kmeans.h"
#include <cmath>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <unordered_set>

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
    // Choose first centroid randomly
    std::uniform_int_distribution<> dis(0, data_points.size() - 1);
    clusters[0].centroid = data_points[dis(rng)];
    
    // Choose remaining centroids using k-means++
    for (uint32_t cluster_idx = 1; cluster_idx < k; ++cluster_idx) {
        std::vector<T> distances(data_points.size());
        T total_distance = 0;
        
        // Compute distances to nearest existing centroid
        for (size_t point_idx = 0; point_idx < data_points.size(); ++point_idx) {
            T min_distance = std::numeric_limits<T>::max();
            
            for (uint32_t existing_cluster = 0; existing_cluster < cluster_idx; ++existing_cluster) {
                T distance = metrics::EuclideanDistance(data_points[point_idx], 
                                                       clusters[existing_cluster].centroid,
                                                       data_points[point_idx].size());
                min_distance = std::min(min_distance, distance);
            }
            
            distances[point_idx] = min_distance * min_distance; // Squared distance
            total_distance += distances[point_idx];
        }
        
        // Choose next centroid with probability proportional to squared distance
        std::uniform_real_distribution<T> prob_dis(0, total_distance);
        T target = prob_dis(rng);
        
        T cumulative = 0;
        for (size_t point_idx = 0; point_idx < data_points.size(); ++point_idx) {
            cumulative += distances[point_idx];
            if (cumulative >= target) {
                clusters[cluster_idx].centroid = data_points[point_idx];
                break;
            }
        }
    }
}

template <typename T>
void KMeans<T>::assign_points_to_clusters() {
    // Clear existing assignments
    for (auto& cluster : clusters) {
        cluster.clear_points();
    }
    
    // Assign each point to nearest cluster
    for (size_t point_idx = 0; point_idx < data_points.size(); ++point_idx) {
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
        
        // Calculate new centroid
        std::vector<T> new_centroid(dimensions, 0);
        for (int point_idx : cluster.point_indices) {
            for (size_t dim = 0; dim < dimensions; ++dim) {
                new_centroid[dim] += data_points[point_idx][dim];
            }
        }
        
        for (size_t dim = 0; dim < dimensions; ++dim) {
            new_centroid[dim] /= cluster.size();
        }
        
        // Check for convergence
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
    
    // Main k-means loop
    for (uint32_t iteration = 0; iteration < max_iterations; ++iteration) {
        assign_points_to_clusters();
        
        if (update_centroids()) {
            std::cout << "K-means converged after " << iteration + 1 << " iterations" << std::endl;
            return true;
        }
    }
    
    std::cout << "K-means reached maximum iterations (" << max_iterations << ")" << std::endl;
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
    T inertia = 0;
    
    for (const auto& cluster : clusters) {
        for (int point_idx : cluster.point_indices) {
            T distance = metrics::EuclideanDistance(data_points[point_idx], cluster.centroid, data_points[point_idx].size());
            inertia += distance * distance;
        }
    }
    
    return inertia;
}

template <typename T>
T KMeans<T>::get_silhouette_score() const {
    if (clusters.size() < 2) return 0;
    
    std::vector<int> labels = get_labels();
    T total_silhouette = 0;
    int valid_points = 0;
    
    for (size_t i = 0; i < data_points.size(); ++i) {
        int cluster_a = labels[i];
        if (clusters[cluster_a].size() <= 1) continue;
        
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
            T silhouette = (b - a) / std::max(a, b);
            total_silhouette += silhouette;
            valid_points++;
        }
    }
    
    return valid_points > 0 ? total_silhouette / valid_points : 0;
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