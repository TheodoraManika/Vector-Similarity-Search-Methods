#include "Hypercube.h"
#include "../../utils/math/math.h"
#include <algorithm>
#include <queue>
#include <unordered_set>
#include <cmath>
#include <random>
#include <unordered_map>
#include <limits>
#include <list>
#include <functional>

// Constructor
template <typename T>
Hypercube<T>::Hypercube(const uint32_t kproj, uint64_t m, uint32_t M, uint32_t probes, 
                        uint32_t n_points, double w, uint32_t space_dim, 
                        std::vector<std::vector<T>> init_vectors, uint32_t seed)
    : kproj(kproj), m(m), M(M), probes(probes), n_points(n_points), w(w),
      space_dim(space_dim), seed(seed), feature_vectors(init_vectors) {

    // number of buckets: 2^kproj
    n_buckets = (kproj >= 31) ? 0 : (1u << kproj); // guard overflow; kproj is typically small
    if (n_buckets == 0) n_buckets = 1u << std::min<uint32_t>(kproj, 20); // fallback

    // RNGs
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> dis01(0.0, 1.0);
    std::normal_distribution<double> gauss(0.0, 1.0);

    // init kproj LSH hash fns (a ~ N(0,1), t ~ U[0,w))
    hash_fns.clear();
    for (uint32_t i = 0; i < kproj; ++i) {
        our_math::lsh::HashFunction hf;
        hf.a.resize(space_dim);
        for (uint32_t d = 0; d < space_dim; ++d) hf.a[d] = gauss(gen);
        hf.t = dis01(gen) * w;
        hash_fns.push_back(hf);
    }

    // pre-map f-coins for each hash function and each possible bucket index
    flipped_coins.assign(kproj, std::vector<bool>(n_buckets, false));
    for (uint32_t i = 0; i < kproj; ++i) {
        std::mt19937 g(seed + 1337u * i);
        std::uniform_int_distribution<int> bit01(0, 1);
        for (uint32_t b = 0; b < n_buckets; ++b) {
            flipped_coins[i][b] = (bit01(g) == 1);
        }
    }

    // dense hypercube table: 2^kproj buckets (lists)
    hypercube.assign(n_buckets, std::list<int>());

    // insert all points
    for (size_t idx = 0; idx < feature_vectors.size(); ++idx) {
        int bucket = hash_to_hypercube(feature_vectors[idx]);
        if (bucket >= 0 && static_cast<uint32_t>(bucket) < n_buckets) {
            hypercube[static_cast<uint32_t>(bucket)].push_back(static_cast<int>(idx));
        }
    }
}

template <typename T>
Hypercube<T>::~Hypercube() {}

// Build vertex id using f-mapped bits; reduce hash to [0..n_buckets-1]
template <typename T>
int Hypercube<T>::hash_to_hypercube(std::vector<T> vec) {
    uint32_t vertex = 0;
    auto it = hash_fns.begin();
    for (uint32_t i = 0; i < kproj; ++i, ++it) {
        int h_raw = our_math::computeBasicHash(vec, *it, static_cast<uint32_t>(w));
        uint32_t h_idx = (n_buckets ? (static_cast<uint32_t>(h_raw) % n_buckets) : 0u);
        bool bit = flipped_coins[i][h_idx];
        if (bit) vertex |= (1u << i);
    }
    return static_cast<int>(vertex);
}

template <typename T>
std::vector<uint32_t> Hypercube<T>::neighbors_by_hamming(uint32_t vertex, uint32_t dist) const {
    std::vector<uint32_t> result;
    if (dist == 0) {
        result.push_back(vertex);
        return result;
    }
    if (dist > kproj) return result; // Can't have Hamming distance > kproj bits
    
    // Generate all combinations of 'dist' bit positions to flip
    // Use recursive generation or iterative with bit manipulation
    std::vector<uint32_t> bit_positions;
    for (uint32_t i = 0; i < kproj; ++i) {
        bit_positions.push_back(i);
    }
    
    // Generate all combinations of 'dist' positions from kproj bits
    std::function<void(uint32_t, uint32_t, uint32_t)> generate_combinations;
    generate_combinations = [&](uint32_t start, uint32_t chosen, uint32_t mask) {
        if (chosen == dist) {
            result.push_back(vertex ^ mask); // Flip the chosen bits
            return;
        }
        for (uint32_t i = start; i <= kproj - (dist - chosen); ++i) {
            generate_combinations(i + 1, chosen + 1, mask | (1u << i));
        }
    };
    
    generate_combinations(0, 0, 0);
    return result;
}

template <typename T>
std::pair<int,T> Hypercube<T>::NearestNeighbour(std::vector<T> query_vector) {
    auto lst = kNearestNeighbour(query_vector, 1);
    if (lst.empty()) return {-1, std::numeric_limits<T>::max()};
    return lst.front();
}

template <typename T>
std::list<std::pair<int,T>> Hypercube<T>::kNearestNeighbour(std::vector<T> query_vector, int k) {
    std::list<std::pair<int,T>> result;
    T kth_min = std::numeric_limits<T>::max();

    uint32_t visited_pts = 0;
    uint32_t probes_checked = 0;

    int q_bucket = hash_to_hypercube(query_vector);
    if (q_bucket < 0) return result;

    auto scan_bucket = [&](uint32_t b_idx) {
        if (visited_pts >= M) return;
        for (int pid : hypercube[b_idx]) {
            if (visited_pts >= M) break;
            ++visited_pts;

            T dist = metrics::ManhattanDistance(feature_vectors[pid], query_vector, space_dim);
            std::pair<int,T> p{pid, dist};

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

    // Step 1: scan query bucket
    scan_bucket(static_cast<uint32_t>(q_bucket));
    ++probes_checked;
    if (visited_pts >= M || probes_checked >= probes) return result;

    // Step 2: expand by increasing Hamming distance shells using efficient generation
    for (uint32_t shell = 1; probes_checked < probes && visited_pts < M; ++shell) {
        // Generate only buckets at exact Hamming distance 'shell' from query bucket
        std::vector<uint32_t> neighbors = neighbors_by_hamming(static_cast<uint32_t>(q_bucket), shell);
        
        for (uint32_t neighbor_bucket : neighbors) {
            if (probes_checked >= probes || visited_pts >= M) break;
            scan_bucket(neighbor_bucket);
            ++probes_checked;
        }
    }

    return result;
}

template <typename T>
std::list<std::pair<int,T>> Hypercube<T>::RangeSearch(std::vector<T> query_vector, double radius, int c) {
    std::list<std::pair<int,T>> out;
    uint32_t visited_pts = 0;
    uint32_t probes_checked = 0;

    int q_bucket = hash_to_hypercube(query_vector);
    if (q_bucket < 0) return out;

    auto collect_bucket = [&](uint32_t b_idx) {
        if (visited_pts >= M) return;
        for (int pid : hypercube[b_idx]) {
            if (visited_pts >= M) break;
            ++visited_pts;
            T dist = metrics::ManhattanDistance(feature_vectors[pid], query_vector, space_dim);
            if (dist < static_cast<T>(radius * c)) out.emplace_back(pid, dist);
        }
    };

    collect_bucket(static_cast<uint32_t>(q_bucket));
    ++probes_checked;
    if (visited_pts >= M || probes_checked >= probes) return out;

    // Expand by Hamming distance shells using efficient generation
    for (uint32_t shell = 1; probes_checked < probes && visited_pts < M; ++shell) {
        std::vector<uint32_t> neighbors = neighbors_by_hamming(static_cast<uint32_t>(q_bucket), shell);
        
        for (uint32_t neighbor_bucket : neighbors) {
            if (probes_checked >= probes || visited_pts >= M) break;
            collect_bucket(neighbor_bucket);
            ++probes_checked;
        }
    }
    return out;
}

// keep original API
template <typename T>
std::list<std::pair<int,T>> Hypercube<T>::RangeSearch(std::vector<T> query_vector, double radius) {
    return RangeSearch(query_vector, radius, 1);
}

// Explicit template instantiation
template class Hypercube<float>;
template class Hypercube<double>;