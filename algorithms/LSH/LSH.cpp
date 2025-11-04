#include "LSH.h"
#include <algorithm>
#include <climits>
#include <random>
#include <functional>
#include <queue>
#include <unordered_set>
#include <stdexcept>
#include <cmath>

template <typename T>
LSH<T>::LSH(uint32_t L, uint64_t m, uint32_t M, uint32_t n_points, 
            uint32_t k, uint32_t space_dim, uint32_t w, 
            std::vector<std::vector<T>> init_vectors, int seed)
    : L(L), m(m), M(M), n_points(n_points), k(k), 
      space_dim(space_dim), w(w), feature_vectors(std::move(init_vectors)),
      r1(0.0), r2(0.0), P1(0.0), P2(0.0) {
    
    // Note: m should be prime M = 2^32 - 5 = 4294967291 for classic hash function
    // Note: M (TableSize) heuristic: n/4, n/8, or n/16 for minimizing empty buckets
    
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
    
    // Use prime table size for better hash distribution
    this->M = our_math::next_prime(M);
    
    // Validate that all vectors have the correct dimension
    for (const auto& vec : feature_vectors) {
        if (vec.size() != space_dim) {
            throw std::invalid_argument("All vectors must have the same dimension as space_dim");
        }
    }
    
    // Initialize random number generator with seed (default seed = 1)
    std::mt19937 gen(seed);
    std::uniform_int_distribution<uint32_t> r_dist(1, this->M - 1); // Random ri coefficients in [1, M)
    
    // Create L amplified hash functions
    for (uint32_t i = 0; i < L; ++i) {
        our_math::lsh::AmplifiedHashFunction amp_fn;
        
        // Create k basic hash functions for this amplified function
        for (uint32_t j = 0; j < k; ++j) {
            our_math::lsh::BasicHashFunction basic_fn;
            basic_fn.a.resize(space_dim);
            
            // Initialize v ~ N(0,1)^d: d-vector with i.i.d. standard normal coordinates
            // vi ~ N(μ=0, σ²=1) for i=1,2,...,d
            // Standard normal: mean μ=0, variance σ²=1, standard deviation σ=1
            std::normal_distribution<double> gauss(0.0, 1.0); // N(mean=0, stddev=1)
            for (uint32_t d = 0; d < space_dim; ++d) {
                basic_fn.a[d] = gauss(gen); // Each vi independently drawn from N(0,1)
            }
            
            // Random shift t uniformly in [0, w)
            std::uniform_real_distribution<double> t_dist(0.0, static_cast<double>(w));
            basic_fn.t = t_dist(gen);
            
            // Random coefficient ri for linear combination: g(p) = sum(ri * hi(p)) mod M
            basic_fn.r = r_dist(gen);
            
            amp_fn.hash_functions.push_back(basic_fn);
        }
        
        amplified_hash_fns.push_back(amp_fn);
    }
    
    // Initialize hash tables
    lsh_tables.resize(L);
    
    // Hash all initial vectors into the tables
    // Store both index and ID(p) for efficient querying
    auto amp_it = amplified_hash_fns.begin();
    for (uint32_t table_idx = 0; table_idx < L; ++table_idx, ++amp_it) {
        for (size_t vec_idx = 0; vec_idx < feature_vectors.size(); ++vec_idx) {
            uint32_t hash_val = our_math::computeAmplifiedHash(feature_vectors[vec_idx], *amp_it, w, M);
            uint64_t id = our_math::computeID(feature_vectors[vec_idx], *amp_it, w);
            lsh_tables[table_idx][hash_val].push_back(std::make_pair(static_cast<int>(vec_idx), id));
        }
    }
}

template <typename T>
std::pair<uint32_t, uint32_t> LSH<T>::computeOptimalParameters(
    double P1, double P2, double target_success_prob) {
    
    if (P1 <= P2 || P1 <= 0 || P1 > 1 || P2 <= 0 || P2 >= 1) {
        throw std::invalid_argument("Invalid P1, P2 values. Need 0 < P2 < P1 <= 1");
    }
    
    // Optimal k minimizes query time
    // rho = log(P1) / log(P2)
    // We want P2^k to be very small (low false positive rate)
    
    // k should be chosen such that we have good discrimination
    // Typical choice: k = log(1 - target_success_prob) / log(1 - P1)
    uint32_t k = static_cast<uint32_t>(std::ceil(std::log(1.0 - target_success_prob) / std::log(1.0 - P1)));
    
    // L should be chosen such that 1 - (1 - P1^k)^L >= target_success_prob
    // Solving: L = log(1 - target_success_prob) / log(1 - P1^k)
    double P1_k = std::pow(P1, static_cast<double>(k));
    uint32_t L = static_cast<uint32_t>(std::ceil(std::log(1.0 - target_success_prob) / std::log(1.0 - P1_k)));
    
    // Ensure minimum values
    k = std::max(k, 1u);
    L = std::max(L, 1u);
    
    return std::make_pair(k, L);
}

template <typename T>
LSH<T>::LSH(double r1, double r2, double P1, double P2,
            uint32_t space_dim, uint32_t w,
            std::vector<std::vector<T>> init_vectors,
            double target_success_prob, int seed)
    : space_dim(space_dim), w(w), feature_vectors(std::move(init_vectors)),
      r1(r1), r2(r2), P1(P1), P2(P2) {
    
    // Validate LSH parameters
    if (r1 >= r2) {
        throw std::invalid_argument("r1 must be less than r2");
    }
    if (P1 <= P2) {
        throw std::invalid_argument("P1 must be greater than P2");
    }
    if (r1 <= 0 || r2 <= 0) {
        throw std::invalid_argument("r1 and r2 must be positive");
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
    
    // Compute optimal k and L based on collision probabilities
    auto [optimal_k, optimal_L] = computeOptimalParameters(P1, P2, target_success_prob);
    
    this->k = optimal_k;
    this->L = optimal_L;
    
    // Compute M (TableSize) based on expected number of points
    // Classic heuristic: TableSize = n/4, n/8, or n/16
    // Using n/8 for balanced performance (not too many empty buckets, reasonable size)
    this->n_points = static_cast<uint32_t>(feature_vectors.size());
    uint32_t desired_M = std::max(n_points / 8, 100u); // TableSize = n/8 heuristic
    this->M = our_math::next_prime(desired_M); // Use prime for better distribution
    this->m = 4294967291ULL; // Prime M = 2^32 - 5 for double modulo
    
    std::cout << "LSH Parameters computed from (r1=" << r1 << ", r2=" << r2 
              << ", P1=" << P1 << ", P2=" << P2 << "):" << std::endl;
    std::cout << "  k (hash functions per table): " << k << std::endl;
    std::cout << "  L (number of tables): " << L << std::endl;
    std::cout << "  M (table size): " << M << std::endl;
    
    // Validate that all vectors have the correct dimension
    for (const auto& vec : feature_vectors) {
        if (vec.size() != space_dim) {
            throw std::invalid_argument("All vectors must have dimension " + std::to_string(space_dim));
        }
    }
    
    // Initialize random number generator with seed (default seed = 1)
    std::mt19937 gen(seed);
    std::uniform_int_distribution<uint32_t> r_dist(1, this->M - 1); // Random ri coefficients in [1, M)
    
    // Create L amplified hash functions
    for (uint32_t i = 0; i < L; ++i) {
        our_math::lsh::AmplifiedHashFunction amp_fn;
        
        // Create k basic hash functions for this amplified function
        for (uint32_t j = 0; j < k; ++j) {
            our_math::lsh::BasicHashFunction basic_fn;
            basic_fn.a.resize(space_dim);
            
            // Initialize v ~ N(0,1)^d: d-vector with i.i.d. standard normal coordinates
            // vi ~ N(μ=0, σ²=1) for i=1,2,...,d
            // Standard normal: mean μ=0, variance σ²=1, standard deviation σ=1
            std::normal_distribution<double> gauss(0.0, 1.0); // N(mean=0, stddev=1)
            for (uint32_t d = 0; d < space_dim; ++d) {
                basic_fn.a[d] = gauss(gen); // Each vi independently drawn from N(0,1)
            }
            
            // Random shift t uniformly in [0, w)
            std::uniform_real_distribution<double> t_dist(0.0, static_cast<double>(w));
            basic_fn.t = t_dist(gen);
            
            // Random coefficient ri for linear combination: g(p) = sum(ri * hi(p)) mod M
            basic_fn.r = r_dist(gen);
            
            amp_fn.hash_functions.push_back(basic_fn);
        }
        
        amplified_hash_fns.push_back(amp_fn);
    }
    
    // Initialize hash tables
    lsh_tables.resize(L);
    
    // Hash all initial vectors into the tables
    // Store both index and ID(p) for efficient querying
    auto amp_it = amplified_hash_fns.begin();
    for (uint32_t table_idx = 0; table_idx < L; ++table_idx, ++amp_it) {
        for (size_t vec_idx = 0; vec_idx < feature_vectors.size(); ++vec_idx) {
            uint32_t hash_val = our_math::computeAmplifiedHash(feature_vectors[vec_idx], *amp_it, w, M);
            uint64_t id = our_math::computeID(feature_vectors[vec_idx], *amp_it, w);
            lsh_tables[table_idx][hash_val].push_back(std::make_pair(static_cast<int>(vec_idx), id));
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
    
    // Print LSH sensitivity parameters if they were set
    if (r1 > 0 && r2 > 0 && P1 > 0 && P2 > 0) {
        std::cout << "LSH Sensitivity Parameters:" << std::endl;
        std::cout << "  r1 (near threshold): " << r1 << std::endl;
        std::cout << "  r2 (far threshold): " << r2 << std::endl;
        std::cout << "  P1 (near collision prob): " << P1 << std::endl;
        std::cout << "  P2 (far collision prob): " << P2 << std::endl;
        std::cout << std::endl;
    }
    
    std::cout << "Hash Structure:" << std::endl;
    std::cout << "  Number of hash tables (L): " << L << std::endl;
    std::cout << "  Hash functions per table (k): " << k << std::endl;
    std::cout << "  Hash table size (M): " << M << std::endl;
    std::cout << "  Total vectors: " << feature_vectors.size() << std::endl;
    std::cout << "  Vector dimension: " << space_dim << std::endl;
    std::cout << "  Window size (w): " << w << std::endl;
    
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
        // Compute hash value and ID for query
        uint32_t hash_val = our_math::computeAmplifiedHash(query_vector, *amp_it, w, M);
        // Note: We intentionally do NOT filter candidates by ID equality anymore
        // because it was too strict and filtered out correct neighbors.
        
        // Get candidates from this bucket
        auto bucket_it = lsh_tables[table_idx].find(hash_val);
        if (bucket_it != lsh_tables[table_idx].end()) {
            const std::vector<std::pair<int, uint64_t>>& candidates = bucket_it->second;
        
            for (const auto& candidate : candidates) {
                int candidate_idx = candidate.first;
                
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
    
    std::list<std::pair<int, T>> result;
    T kth_min = std::numeric_limits<T>::max();
    std::unordered_set<int> checked_indices;
    
    auto scan_bucket = [&](const std::vector<std::pair<int, uint64_t>>& candidates) {
        for (const auto& candidate : candidates) {
            int pid = candidate.first;
            
            // Skip if already checked (avoid duplicate distance computations)
            if (checked_indices.count(pid)) {
                continue;
            }
            checked_indices.insert(pid);
            
            // Validate candidate index
            if (pid < 0 || pid >= static_cast<int>(feature_vectors.size())) {
                continue; // Skip invalid indices
            }
            
            T dist = metrics::EuclideanDistance(query_vector, feature_vectors[pid], static_cast<int>(query_vector.size()));
            std::pair<int, T> p{pid, dist};
            
            if (result.empty()) {
                result.push_back(p);
                kth_min = dist;
                continue;
            }
            
            // skip duplicates
            if (std::find(result.begin(), result.end(), p) != result.end()) continue;
            
            if (result.size() < static_cast<size_t>(k)) {
                // insert sorted
                auto it = result.begin();
                while (it != result.end() && it->second <= dist) ++it;
                result.insert(it, p);
                kth_min = result.back().second;
            } else if (dist < kth_min) {
                auto it = result.begin();
                while (it != result.end() && it->second <= dist) ++it;
                result.insert(it, p);
                result.pop_back();
                kth_min = result.back().second;
            }
        }
    };
    
    // Scan all L hash tables
    auto amp_it = amplified_hash_fns.begin();
    for (uint32_t table_idx = 0; table_idx < L; ++table_idx, ++amp_it) {
        // Compute hash value for query
        uint32_t hash_val = our_math::computeAmplifiedHash(query_vector, *amp_it, w, M);
        
        // Get candidates from exact bucket only
        auto bucket_it = lsh_tables[table_idx].find(hash_val);
        if (bucket_it != lsh_tables[table_idx].end()) {
            scan_bucket(bucket_it->second);
        }
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
    
    std::list<std::pair<int, T>> out;
    std::unordered_set<int> checked_indices;
    
    auto collect_bucket = [&](const std::vector<std::pair<int, uint64_t>>& candidates) {
        for (const auto& candidate : candidates) {
            int pid = candidate.first;
            
            // Skip if already checked (avoid duplicate distance computations)
            if (checked_indices.count(pid)) {
                continue;
            }
            checked_indices.insert(pid);
            
            // Validate candidate index
            if (pid < 0 || pid >= static_cast<int>(feature_vectors.size())) {
                continue; // Skip invalid indices
            }
            
            T dist = metrics::EuclideanDistance(query_vector, feature_vectors[pid], static_cast<int>(query_vector.size()));
            if (dist < static_cast<T>(radius * c)) {
                out.emplace_back(pid, dist);
            }
        }
    };
    
    // Scan all L hash tables
    auto amp_it = amplified_hash_fns.begin();
    for (uint32_t table_idx = 0; table_idx < L; ++table_idx, ++amp_it) {
        // Compute hash value for query
        uint32_t hash_val = our_math::computeAmplifiedHash(query_vector, *amp_it, w, M);
        
        // Get candidates from exact bucket only
        auto bucket_it = lsh_tables[table_idx].find(hash_val);
        if (bucket_it != lsh_tables[table_idx].end()) {
            collect_bucket(bucket_it->second);
        }
    }
    
    return out;
}

template <typename T>
void LSH<T>::verifyLSHProperty(const std::vector<std::vector<T>>& sample_points,
                               size_t num_samples) const {
    if (!isInitialized() || sample_points.empty()) {
        std::cout << "Cannot verify LSH property: not initialized or no samples" << std::endl;
        return;
    }
    
    if (r1 <= 0 || r2 <= 0 || P1 <= 0 || P2 <= 0) {
        std::cout << "Cannot verify LSH property: sensitivity parameters not set" << std::endl;
        return;
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> point_dist(0, sample_points.size() - 1);
    
    size_t near_collisions = 0;  // Collisions when dist <= r1
    size_t far_collisions = 0;   // Collisions when dist >= r2
    size_t near_samples = 0;
    size_t far_samples = 0;
    
    for (size_t trial = 0; trial < num_samples; ++trial) {
        // Pick two random points
        size_t idx1 = point_dist(gen);
        size_t idx2 = point_dist(gen);
        
        if (idx1 == idx2) continue;
        
        const auto& p = sample_points[idx1];
        const auto& q = sample_points[idx2];
        
        if (p.size() != space_dim || q.size() != space_dim) {
            continue; // Skip mismatched dimensions
        }
        
        // Compute distance
        T dist = metrics::EuclideanDistance(p, q, static_cast<int>(p.size()));
        
        // Pick a random hash function from a random table
        std::uniform_int_distribution<uint32_t> table_dist(0, L - 1);
        uint32_t table_idx = table_dist(gen);
        
        auto amp_it = amplified_hash_fns.begin();
        std::advance(amp_it, table_idx);
        
        uint32_t hash_p = our_math::computeAmplifiedHash(p, *amp_it, w, M);
        uint32_t hash_q = our_math::computeAmplifiedHash(q, *amp_it, w, M);
        
        bool collision = (hash_p == hash_q);
        
        if (dist <= r1) {
            near_samples++;
            if (collision) near_collisions++;
        } else if (dist >= r2) {
            far_samples++;
            if (collision) far_collisions++;
        }
    }
    
    double empirical_P1 = near_samples > 0 ? static_cast<double>(near_collisions) / near_samples : 0.0;
    double empirical_P2 = far_samples > 0 ? static_cast<double>(far_collisions) / far_samples : 0.0;
    
    std::cout << "\n=== LSH Property Verification ===" << std::endl;
    std::cout << "Expected (r1=" << r1 << ", r2=" << r2 << ", P1=" << P1 << ", P2=" << P2 << ")" << std::endl;
    std::cout << "Empirical results from " << num_samples << " trials:" << std::endl;
    std::cout << "  Near pairs (dist <= r1): " << near_samples 
              << ", collisions: " << near_collisions 
              << " (" << (empirical_P1 * 100) << "%)" << std::endl;
    std::cout << "  Expected P1: " << (P1 * 100) << "%, Empirical: " << (empirical_P1 * 100) << "%" << std::endl;
    std::cout << "  Far pairs (dist >= r2): " << far_samples 
              << ", collisions: " << far_collisions 
              << " (" << (empirical_P2 * 100) << "%)" << std::endl;
    std::cout << "  Expected P2: " << (P2 * 100) << "%, Empirical: " << (empirical_P2 * 100) << "%" << std::endl;
    
    bool property_holds = (empirical_P1 >= P1 * 0.8) && (empirical_P2 <= P2 * 1.2);
    std::cout << "  LSH Property: " << (property_holds ? "✓ HOLDS" : "✗ VIOLATED") << std::endl;
    std::cout << "================================\n" << std::endl;
}

// Explicit template instantiations
template class LSH<float>;
template class LSH<double>;