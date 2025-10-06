#include "LSH.h"
#include <algorithm>
#include <climits>
#include <random>
#include <functional>
#include <queue>
#include <unordered_set>
#include <stdexcept>

template <typename T>
LSH<T>::LSH(uint32_t L, uint64_t m, uint32_t M, uint32_t n_points, 
            uint32_t k, uint32_t space_dim, uint32_t w, 
            std::vector<std::vector<T>> init_vectors)
    : L(L), m(m), M(M), n_points(n_points), k(k), w(w), 
      space_dim(space_dim), feature_vectors(std::move(init_vectors)) {
    
    // Validate constructor parameters
    if (L == 0) {
        throw std::invalid_argument("Number of hash tables (L) must be positive");
    }
    if (k == 0) {
        throw std::invalid_argument("Number of hash functions (k) must be positive");
    }
    if (M == 0) {
        throw std::invalid_argument("Hash table size (M) must be positive");
    }
    if (space_dim == 0) {
        throw std::invalid_argument("Space dimension must be positive");
    }
    if (w == 0) {
        throw std::invalid_argument("Window size (w) must be positive");
    }
    if (feature_vectors.empty()) {
        throw std::invalid_argument("Initial vectors cannot be empty");
    }
    
    // Validate that all vectors have the correct dimension
    for (const auto& vec : feature_vectors) {
        if (vec.size() != space_dim) {
            throw std::invalid_argument("All vectors must have the same dimension as space_dim");
        }
    }
    
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    // Create L amplified hash functions
    for (uint32_t i = 0; i < L; ++i) {
        our_math::lsh::AmplifiedHashFunction amp_fn;
        
        // Create k basic hash functions for this amplified function
        for (uint32_t j = 0; j < k; ++j) {
            our_math::lsh::BasicHashFunction basic_fn;
            basic_fn.a.resize(space_dim);
            
            // Initialize coefficients from Gaussian distribution
            std::normal_distribution<double> gauss(0.0, 1.0);
            for (uint32_t d = 0; d < space_dim; ++d) {
                basic_fn.a[d] = gauss(gen);
            }
            
            // Random shift
            basic_fn.t = dis(gen) * w;
            
            amp_fn.hash_functions.push_back(basic_fn);
        }
        
        amplified_hash_fns.push_back(amp_fn);
    }
    
    // Initialize hash tables
    lsh_tables.resize(L);
    
    // Hash all initial vectors into the tables
    auto amp_it = amplified_hash_fns.begin();
    for (uint32_t table_idx = 0; table_idx < L; ++table_idx, ++amp_it) {
        for (size_t vec_idx = 0; vec_idx < feature_vectors.size(); ++vec_idx) {
            uint32_t hash_val = our_math::computeAmplifiedHash(feature_vectors[vec_idx], *amp_it, w, M);
            lsh_tables[table_idx][hash_val].push_back(static_cast<int>(vec_idx));
        }
    }
}

template <typename T>
LSH<T>::~LSH() {
    // Hash tables will automatically clean up
}

template <typename T>
void LSH<T>::validateInput(const std::vector<T>& query_vector) const {
    if (query_vector.empty()) {
        throw std::invalid_argument("Query vector cannot be empty");
    }
    if (query_vector.size() != space_dim) {
        throw std::invalid_argument("Query vector dimension mismatch");
    }
}

template <typename T>
bool LSH<T>::isInitialized() const {
    return !feature_vectors.empty() && !amplified_hash_fns.empty() && 
           lsh_tables.size() == L;
}

template <typename T>
void LSH<T>::printStatistics() const {
    if (!isInitialized()) {
        std::cout << "LSH not initialized" << std::endl;
        return;
    }
    
    std::cout << "=== LSH Statistics ===" << std::endl;
    std::cout << "Number of hash tables (L): " << L << std::endl;
    std::cout << "Hash functions per table (k): " << k << std::endl;
    std::cout << "Hash table size (M): " << M << std::endl;
    std::cout << "Total vectors: " << feature_vectors.size() << std::endl;
    std::cout << "Vector dimension: " << space_dim << std::endl;
    std::cout << "Window size (w): " << w << std::endl;
    
    // Analyze hash table distribution
    size_t total_buckets_used = 0;
    size_t total_collisions = 0;
    size_t max_bucket_size = 0;
    
    for (uint32_t i = 0; i < L; ++i) {
        size_t buckets_used = lsh_tables[i].size();
        total_buckets_used += buckets_used;
        
        for (const auto& bucket : lsh_tables[i]) {
            size_t bucket_size = bucket.second.size();
            if (bucket_size > 1) {
                total_collisions += bucket_size - 1;
            }
            max_bucket_size = std::max(max_bucket_size, bucket_size);
        }
    }
    
    double avg_buckets_per_table = static_cast<double>(total_buckets_used) / L;
    double load_factor = static_cast<double>(total_buckets_used) / (L * M);
    
    std::cout << "Average buckets used per table: " << avg_buckets_per_table << std::endl;
    std::cout << "Load factor: " << load_factor << std::endl;
    std::cout << "Total collisions: " << total_collisions << std::endl;
    std::cout << "Max bucket size: " << max_bucket_size << std::endl;
    std::cout << "======================" << std::endl;
}

template <typename T>
std::pair<int, T> LSH<T>::NearestNeighbour(const std::vector<T>& query_vector) {
    validateInput(query_vector);
    
    if (!isInitialized()) {
        throw std::runtime_error("LSH not properly initialized");
    }
    
    int best_idx = -1;
    T best_distance = std::numeric_limits<T>::max();
    std::unordered_set<int> checked_indices;
    
    auto amp_it = amplified_hash_fns.begin();
    for (uint32_t table_idx = 0; table_idx < L; ++table_idx, ++amp_it) {
        // Compute hash value for query
        uint32_t hash_val = our_math::computeAmplifiedHash(query_vector, *amp_it, w, M);
        
        // Get candidates from this bucket
        auto bucket_it = lsh_tables[table_idx].find(hash_val);
        if (bucket_it != lsh_tables[table_idx].end()) {
            const std::vector<int>& candidates = bucket_it->second;
        
            for (int candidate_idx : candidates) {
                // Skip if already checked (O(1) average case with unordered_set)
                if (checked_indices.count(candidate_idx)) {
                    continue;
                }
                checked_indices.insert(candidate_idx);
                
                // Validate candidate index
                if (candidate_idx < 0 || candidate_idx >= static_cast<int>(feature_vectors.size())) {
                    continue; // Skip invalid indices
                }
                
                // Compute actual distance
                T distance = metrics::EuclideanDistance(query_vector, feature_vectors[candidate_idx], static_cast<int>(query_vector.size()));
                
                if (distance < best_distance) {
                    best_distance = distance;
                    best_idx = candidate_idx;
                }
            }
        }
    }
    
    // If no candidates found, perform brute force search on a subset
    if (best_idx == -1 && !feature_vectors.empty()) {
        // Search through first few vectors as fallback
        int search_limit = std::min(static_cast<int>(feature_vectors.size()), 100);
        for (int i = 0; i < search_limit; ++i) {
            T distance = metrics::EuclideanDistance(query_vector, feature_vectors[i], static_cast<int>(query_vector.size()));
            if (distance < best_distance) {
                best_distance = distance;
                best_idx = i;
            }
        }
    }
    
    return std::make_pair(best_idx, best_distance);
}

template <typename T>
std::list<std::pair<int, T>> LSH<T>::kNearestNeighbour(const std::vector<T>& query_vector, int k) {
    validateInput(query_vector);
    
    if (!isInitialized()) {
        throw std::runtime_error("LSH not properly initialized");
    }
    
    if (k <= 0) {
        throw std::invalid_argument("k must be positive");
    }
    
    // Use priority queue to keep track of k best candidates
    std::priority_queue<std::pair<T, int>> pq; // max heap by distance
    std::unordered_set<int> checked_indices;
    
    auto amp_it = amplified_hash_fns.begin();
    for (uint32_t table_idx = 0; table_idx < L; ++table_idx, ++amp_it) {
        // Compute hash value for query
        uint32_t hash_val = our_math::computeAmplifiedHash(query_vector, *amp_it, w, M);
        
        // Get candidates from this bucket
        auto bucket_it = lsh_tables[table_idx].find(hash_val);
        if (bucket_it != lsh_tables[table_idx].end()) {
            const std::vector<int>& candidates = bucket_it->second;
        
            for (int candidate_idx : candidates) {
                // Skip if already checked (O(1) average case with unordered_set)
                if (checked_indices.count(candidate_idx)) {
                    continue;
                }
                checked_indices.insert(candidate_idx);
                
                // Validate candidate index
                if (candidate_idx < 0 || candidate_idx >= static_cast<int>(feature_vectors.size())) {
                    continue; // Skip invalid indices
                }
                
                // Compute actual distance
                T distance = metrics::EuclideanDistance(query_vector, feature_vectors[candidate_idx], static_cast<int>(query_vector.size()));
                
                if (pq.size() < static_cast<size_t>(k)) {
                    pq.push(std::make_pair(distance, candidate_idx));
                } else if (distance < pq.top().first) {
                    pq.pop();
                    pq.push(std::make_pair(distance, candidate_idx));
                }
            }
        }
    }
    
    // If we don't have enough candidates, perform brute force search
    if (pq.size() < static_cast<size_t>(k) && !feature_vectors.empty()) {
        // Calculate how many more we need
        int needed = k - static_cast<int>(pq.size());
        T worst_distance = pq.empty() ? std::numeric_limits<T>::max() : pq.top().first;
        
        // Search through vectors not yet checked
        for (size_t i = 0; i < feature_vectors.size() && needed > 0; ++i) {
            if (checked_indices.count(static_cast<int>(i))) {
                continue; // Skip already checked
            }
            
            T distance = metrics::EuclideanDistance(query_vector, feature_vectors[i], static_cast<int>(query_vector.size()));
            
            if (pq.size() < static_cast<size_t>(k)) {
                pq.push(std::make_pair(distance, static_cast<int>(i)));
                needed--;
            } else if (distance < worst_distance) {
                pq.pop();
                pq.push(std::make_pair(distance, static_cast<int>(i)));
                worst_distance = pq.top().first;
            }
        }
    }
    
    // Convert priority queue to sorted list
    std::list<std::pair<int, T>> result;
    while (!pq.empty()) {
        auto& top = pq.top();
        result.push_front(std::make_pair(top.second, top.first));
        pq.pop();
    }
    
    return result;
}

template <typename T>
std::list<std::pair<int, T>> LSH<T>::RangeSearch(const std::vector<T>& query_vector, double radius, int c) {
    validateInput(query_vector);
    
    if (!isInitialized()) {
        throw std::runtime_error("LSH not properly initialized");
    }
    
    if (radius < 0 || c <= 0) {
        throw std::invalid_argument("Invalid radius or c parameter");
    }
    
    std::list<std::pair<int, T>> result;
    std::unordered_set<int> checked_indices;
    T range_threshold = static_cast<T>(radius * c);
    
    auto amp_it = amplified_hash_fns.begin();
    for (uint32_t table_idx = 0; table_idx < L; ++table_idx, ++amp_it) {
        // Compute hash value for query
        uint32_t hash_val = our_math::computeAmplifiedHash(query_vector, *amp_it, w, M);
        
        // Get candidates from this bucket
        auto bucket_it = lsh_tables[table_idx].find(hash_val);
        if (bucket_it != lsh_tables[table_idx].end()) {
            const std::vector<int>& candidates = bucket_it->second;
        
            for (int candidate_idx : candidates) {
                // Skip if already checked (O(1) average case with unordered_set)
                if (checked_indices.count(candidate_idx)) {
                    continue;
                }
                checked_indices.insert(candidate_idx);
                
                // Validate candidate index
                if (candidate_idx < 0 || candidate_idx >= static_cast<int>(feature_vectors.size())) {
                    continue; // Skip invalid indices
                }
                
                // Compute actual distance
                T distance = metrics::EuclideanDistance(query_vector, feature_vectors[candidate_idx], static_cast<int>(query_vector.size()));
                
                if (distance <= range_threshold) {
                    result.push_back(std::make_pair(candidate_idx, distance));
                }
            }
        }
    }
    
    // Sort by distance
    result.sort([](const std::pair<int, T>& a, const std::pair<int, T>& b) {
        return a.second < b.second;
    });
    
    return result;
}

// Explicit template instantiations
template class LSH<float>;
template class LSH<double>;