#pragma once

#include <vector>
#include <string>
#include <map>
#include <list>
#include <chrono>

// Structure to hold benchmark results
template <typename T>
struct BenchmarkResult {
    std::string algorithm_name;
    std::string dataset_name;
    std::map<std::string, std::string> parameters;
    T recall_at_n;
    T mean_approximation_factor;
    T mean_query_time;
    T queries_per_second;
    size_t dataset_size;
    size_t query_count;
    
    BenchmarkResult() : recall_at_n(0), mean_approximation_factor(0), 
                       mean_query_time(0), queries_per_second(0), 
                       dataset_size(0), query_count(0) {}
};

// Dataset information structure
template <typename T>
struct DatasetInfo {
    std::vector<std::vector<T>> data;
    std::vector<std::vector<T>> queries;
    std::string dataset_name;
    size_t dimensions;
    
    DatasetInfo() : dimensions(0) {}
};

// Algorithm comparison class
template <typename T>
class AlgorithmComparator {
private:
    DatasetInfo<T> dataset_info;
    std::vector<BenchmarkResult<T>> results;
    
public:
    AlgorithmComparator() = default;
    ~AlgorithmComparator() = default;
    
    // Dataset management
    bool load_dataset(const std::string& data_file, const std::string& query_file, const std::string& dataset_type);
    
    // Ground truth computation
    std::vector<std::pair<int, T>> compute_true_nearest_neighbors(const std::vector<T>& query, uint32_t k) const;
    
    // Evaluation metrics
    T compute_recall_at_n(const std::vector<std::vector<int>>& approx_results,
                         const std::vector<std::vector<int>>& true_results) const;
    
    T compute_mean_approximation_factor(const std::vector<std::list<std::pair<int, T>>>& approx_results,
                                       const std::vector<std::list<std::pair<int, T>>>& true_results) const;
    
    // Result management
    void add_result(const BenchmarkResult<T>& result);
    const std::vector<BenchmarkResult<T>>& get_results() const { return results; }
    void clear_results() { results.clear(); }
    
    // Export functionality
    void export_results_csv(const std::string& filename) const;
    
private:
    // Helper functions
    bool is_correct_retrieval(const std::vector<int>& approx_neighbors, 
                             const std::vector<int>& true_neighbors) const;
};
