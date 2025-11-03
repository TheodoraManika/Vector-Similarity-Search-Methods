#include "IVFPQ.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <queue>
#include <unordered_set>
#include <omp.h>

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
void ProductQuantizer<T>::train(const std::vector<std::vector<double>>& training_data) {
    std::cout << "  Training Product Quantizer (" << M << " subspaces, " 
              << codebook_size << " centroids each)..." << std::endl;
    
    // Parallelize k-means training across subspaces
    #pragma omp parallel for schedule(dynamic)
    for (uint32_t m = 0; m < M; ++m) {
        #pragma omp critical
        {
            if (m % 4 == 0 || m == M - 1) {
                std::cout << "    Training subspace " << (m + 1) << "/" << M 
                          << " (thread " << omp_get_thread_num() << ")..." << std::flush;
            }
        }
        
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
        
        #pragma omp critical
        {
            if (m % 4 == 0 || m == M - 1) {
                std::cout << " done" << std::endl;
            }
        }
    }
    std::cout << "  Product Quantizer training complete" << std::endl;
}

template <typename T>
std::vector<uint8_t> ProductQuantizer<T>::encode(const std::vector<double>& vector) const {
    std::vector<uint8_t> codes(M);
    
    // Parallelize encoding across subspaces
    #pragma omp parallel for schedule(static)
    for (uint32_t m = 0; m < M; ++m) {
        T min_dist = std::numeric_limits<T>::max();
        uint8_t best_code = 0;
        
        // Extract subvector
        std::vector<double> subvector(vector.begin() + m * subspace_dim,
                               vector.begin() + (m + 1) * subspace_dim);
        
        // Find nearest centroid
        for (uint32_t k = 0; k < codebook_size; ++k) {
            T dist_squared = 0;
            for (uint32_t d = 0; d < subspace_dim; ++d) {
                T diff = subvector[d] - codebooks[m][k][d];
                dist_squared += diff * diff;
            }
            if (dist_squared < min_dist) {
                min_dist = dist_squared;
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
std::vector<std::vector<T>> ProductQuantizer<T>::compute_distance_table(const std::vector<double>& query) const {
    std::vector<std::vector<T>> distance_table(M);
    
    // Parallelize distance table computation across subspaces
    #pragma omp parallel for schedule(static)
    for (uint32_t m = 0; m < M; ++m) {
        distance_table[m].resize(codebook_size);
        
        // Extract query subvector
        std::vector<double> query_subvec(query.begin() + m * subspace_dim,
                                  query.begin() + (m + 1) * subspace_dim);
        
        // Step 2 (Query): Build LUT[i][h] = ||ri(q) - ci,h||² (SQUARED distance)
        // Compute SQUARED distances to all centroids in this subspace
        for (uint32_t h = 0; h < codebook_size; ++h) {
            T dist_squared = 0;
            for (uint32_t d = 0; d < subspace_dim; ++d) {
                T diff = query_subvec[d] - codebooks[m][h][d];
                dist_squared += diff * diff;
            }
            distance_table[m][h] = dist_squared;
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
    std::cout << "Training IVFPQ..." << std::endl;
    
    // Step 1 & 2: Train coarse quantizer on subset (like IVFFlat)
    std::cout << "  Step 1-2: Training coarse quantizer (" << n_clusters << " clusters)..." << std::flush;
    
    // Use subset for k-means training (like IVFFlat)
    size_t n = dataset.size();
    size_t subset_size = static_cast<size_t>(std::sqrt(n));
    subset_size = std::max(subset_size, static_cast<size_t>(n_clusters));
    subset_size = std::min(subset_size, n);
    
    // Create random subset
    std::vector<size_t> indices(n);
    for (size_t i = 0; i < n; ++i) {
        indices[i] = i;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);
    
    std::vector<std::vector<T>> training_subset;
    training_subset.reserve(subset_size);
    for (size_t i = 0; i < subset_size; ++i) {
        training_subset.push_back(dataset[indices[i]]);
    }
    
    coarse_quantizer->set_data(training_subset);
    coarse_quantizer->fit();
    centroids = coarse_quantizer->get_centroids();
    std::cout << " done" << std::endl;
    
    // Step 3-4: Compute residuals for all vectors and prepare for PQ training
    std::cout << "  Step 3-4: Computing residuals for all vectors..." << std::flush;
    std::vector<std::vector<double>> residuals;
    residuals.reserve(dataset.size());
    
    for (const auto& vector : dataset) {
        // Find nearest centroid
        T min_dist = std::numeric_limits<T>::max();
        int nearest_cluster = 0;
        
        for (size_t j = 0; j < centroids.size(); ++j) {
            T dist = metrics::EuclideanDistanceSquared(vector, centroids[j], vector_dim);
            if (dist < min_dist) {
                min_dist = dist;
                nearest_cluster = static_cast<int>(j);
            }
        }
        
        // Step 3: Compute residual r(x) = x - c(x)
        std::vector<double> residual(vector_dim);
        for (uint32_t d = 0; d < vector_dim; ++d) {
            residual[d] = static_cast<double>(vector[d]) - centroids[nearest_cluster][d];
        }
        
        residuals.push_back(residual);
    }
    std::cout << " done" << std::endl;
    
    // Step 5: Train product quantizer on residuals (not original vectors!)
    std::cout << "  Step 5: Training Product Quantizer on residuals..." << std::endl;
    product_quantizer->train(residuals);
}

template <typename T>
void IVFPQ<T>::add_vectors(const std::vector<std::vector<T>>& vectors) {
    std::cout << "  Step 6-8: Encoding and adding vectors to inverted lists..." << std::endl;
    
    for (size_t i = 0; i < vectors.size(); ++i) {
        // Step 2: Assign to nearest coarse cluster c(x)
        T min_dist = std::numeric_limits<T>::max();
        int cluster_id = 0;
        
        for (size_t j = 0; j < centroids.size(); ++j) {
            T dist = metrics::EuclideanDistanceSquared(vectors[i], centroids[j], vector_dim);
            if (dist < min_dist) {
                min_dist = dist;
                cluster_id = static_cast<int>(j);
            }
        }
        
        // Step 3: Compute residual r(x) = x - c(x)
        std::vector<double> residual(vector_dim);
        for (uint32_t d = 0; d < vector_dim; ++d) {
            residual[d] = static_cast<double>(vectors[i][d]) - centroids[cluster_id][d];
        }
        
        // Step 6-7: Encode residual with product quantizer
        // codei(x) = arg minh ||ri(x) - ci,h||²
        auto codes = product_quantizer->encode(residual);
        
        // Step 8: Append (id(x), PQ(x)) to inverted list ILc(x)
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
    // Step 1 (Query): Score centroids - compute ||q - cj||² and select top b (nprobe) clusters
    auto clusters = find_nearest_clusters(query, nprobe);
    
    // Priority queue for k nearest neighbors
    std::priority_queue<std::pair<T, int>> pq;
    
    // Search in selected clusters
    for (int cluster_id : clusters) {
        const auto& inverted_list = inverted_lists[cluster_id];
        
        // Step 2 (Query): For each selected cluster cj
        // Compute residual r(q) = q - cj
        std::vector<double> query_residual(vector_dim);
        for (uint32_t d = 0; d < vector_dim; ++d) {
            query_residual[d] = static_cast<double>(query[d]) - centroids[cluster_id][d];
        }
        
        // Split r(q) = [r1(q), ..., rM(q)]
        // Build LUT[i][h] = ||ri(q) - ci,h||² for i=1,...,M, h=1,...,s
        auto distance_table = product_quantizer->compute_distance_table(query_residual);
        
        // Step 3 (Query): For each x in U (union of probed lists)
        for (const auto& entry : inverted_list) {
            int vec_idx = entry.first;
            const auto& codes = entry.second;  // PQ(x) = [code1(x), ..., codeM(x)]
            
            // Asymmetric Distance Computation (ADC):
            // d(q, x) = Σ(i=1 to M) LUT[i][codei(x)]
            T distance = compute_pq_distance(query_residual, codes, distance_table);
            
            if (pq.size() < k) {
                pq.emplace(distance, vec_idx);
            } else if (distance < pq.top().first) {
                pq.pop();
                pq.emplace(distance, vec_idx);
            }
        }
    }
    
    // Step 4 (Query): Return R nearest points from U
    std::list<std::pair<int, T>> result;
    while (!pq.empty()) {
        result.emplace_front(pq.top().second, pq.top().first);
        pq.pop();
    }
    
    return result;
}

template <typename T>
std::list<std::pair<int, T>> IVFPQ<T>::range_search(const std::vector<T>& query, T radius) const {
    // Step 1 (Query): Score centroids and select top b (nprobe) clusters
    auto clusters = find_nearest_clusters(query, nprobe);
    
    std::list<std::pair<int, T>> result;
    
    // Search in selected clusters
    for (int cluster_id : clusters) {
        const auto& inverted_list = inverted_lists[cluster_id];
        
        // Step 2 (Query): Compute residual r(q) = q - cj and build LUT
        std::vector<double> query_residual(vector_dim);
        for (uint32_t d = 0; d < vector_dim; ++d) {
            query_residual[d] = static_cast<double>(query[d]) - centroids[cluster_id][d];
        }
        
        // Build LUT[i][h] = ||ri(q) - ci,h||²
        auto distance_table = product_quantizer->compute_distance_table(query_residual);
        
        // Step 3 (Query): For each x in U, compute ADC distance
        for (const auto& entry : inverted_list) {
            int vec_idx = entry.first;
            const auto& codes = entry.second;
            
            // ADC: d(q, x) = Σ LUT[i][codei(x)]
            T distance = compute_pq_distance(query_residual, codes, distance_table);
            
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
    cluster_distances.resize(centroids.size());
    
    // Step 1 (Query): Score centroids - compute ||q - cj||² for all j (parallelized)
    // Using squared distance for coarse search (faster, ordering preserved)
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < centroids.size(); ++i) {
        T distance_squared = metrics::EuclideanDistanceSquared(query, centroids[i], vector_dim);
        cluster_distances[i] = std::make_pair(distance_squared, static_cast<int>(i));
    }
    
    // Select top b (nprobe) clusters
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
T IVFPQ<T>::compute_pq_distance([[maybe_unused]] const std::vector<double>& query, const std::vector<uint8_t>& codes,
                                const std::vector<std::vector<T>>& distance_table) const {
    // Step 3 (Query): Asymmetric Distance Computation (ADC)
    // d(q, x) = Σ(i=1 to M) LUT[i][codei(x)]
    // Since LUT contains squared distances, this computes the sum of squared subspace distances
    T total_distance_squared = 0;
    
    for (uint32_t m = 0; m < M; ++m) {
        total_distance_squared += distance_table[m][codes[m]];
    }
    
    // Return Euclidean distance (square root of sum of squared distances)
    return std::sqrt(total_distance_squared);
}

// Explicit template instantiation
template class ProductQuantizer<float>;
template class ProductQuantizer<double>;
template class IVFPQ<float>;
template class IVFPQ<double>;
