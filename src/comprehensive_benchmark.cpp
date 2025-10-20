#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <map>
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <set>
#include <numeric>

// Include algorithm headers
#include "algorithms/LSH/LSH.h"
#include "algorithms/hypercube/Hypercube.h"
#include "algorithms/IVFFlat/IVFFlat.h"
#include "algorithms/IVFPQ/IVFPQ.h"

// Include data processing
#include "dataprocessing/SIFT/SIFT.h"
#include "dataprocessing/MNIST/MNIST.h"
#include "metrics/metrics.h"
#include "utils/helpers/Utils.h"

// Performance metrics structure
struct BenchmarkResult {
    std::string algorithm;
    std::string dataset;
    std::map<std::string, std::string> parameters;
    
    // Performance metrics as required by PDFs
    double mean_approximation_factor;  // μό κλάσμα προσέγγισης
    double recall_at_n;               // ανάκληση στους Ν πλησιέστερους γείτονες
    double queries_per_second;        // πλήθος αναζητήσεων ανά δευτερόλεπτο
    double mean_query_time_ms;        // μέσος χρόνος ερωτήματος
    
    size_t dataset_size;
    size_t query_count;
    size_t k_neighbors;
    double range_radius;
    
    BenchmarkResult() : mean_approximation_factor(0), recall_at_n(0), 
                       queries_per_second(0), mean_query_time_ms(0),
                       dataset_size(0), query_count(0), k_neighbors(0), range_radius(0) {}
};

class ComprehensiveBenchmark {
private:
    std::vector<BenchmarkResult> results;
    
    // Ground truth computation using brute force
    template<typename T>
    std::vector<std::pair<int, T>> computeTrueKNN(const std::vector<T>& query, 
                                                  const std::vector<std::vector<T>>& dataset, 
                                                  int k) {
        std::vector<std::pair<int, T>> distances;
        
        for (size_t i = 0; i < dataset.size(); ++i) {
            T dist = metrics::EuclideanDistance(query, dataset[i], static_cast<int>(query.size()));
            distances.push_back({static_cast<int>(i), dist});
        }
        
        std::sort(distances.begin(), distances.end(), 
                  [](const std::pair<int, T>& a, const std::pair<int, T>& b) {
                      return a.second < b.second;
                  });
        
        distances.resize(std::min(k, static_cast<int>(distances.size())));
        return distances;
    }
    
    // Compute approximation factor
    template<typename T>
    double computeApproximationFactor(const std::list<std::pair<int, T>>& approx_results,
                                     const std::vector<std::pair<int, T>>& true_results) {
        if (approx_results.empty() || true_results.empty()) return 0.0;
        
        double total_af = 0.0;
        size_t count = 0;
        
        auto it = approx_results.begin();
        for (size_t i = 0; i < true_results.size() && it != approx_results.end(); ++it, ++i) {
            if (true_results[i].second > 0) {  // Avoid division by zero
                double af = it->second / true_results[i].second;
                total_af += af;
                count++;
            }
        }
        
        return count > 0 ? total_af / count : 0.0;
    }
    
    // Compute recall@N
    template<typename T>
    double computeRecallAtN(const std::list<std::pair<int, T>>& approx_results,
                           const std::vector<std::pair<int, T>>& true_results) {
        if (approx_results.empty() || true_results.empty()) return 0.0;
        
        std::set<int> true_indices;
        for (const auto& pair : true_results) {
            true_indices.insert(pair.first);
        }
        
        size_t matches = 0;
        for (const auto& pair : approx_results) {
            if (true_indices.count(pair.first)) {
                matches++;
            }
        }
        
        return static_cast<double>(matches) / std::min(approx_results.size(), true_results.size());
    }

public:
    // Test LSH algorithm
    template<typename T>
    BenchmarkResult testLSH(const std::vector<std::vector<T>>& dataset,
                           const std::vector<std::vector<T>>& queries,
                           const std::string& dataset_name,
                           uint32_t k = 4, uint32_t L = 5, uint32_t w = 4,
                           uint32_t N = 5, double R = 2.0) {
        
        BenchmarkResult result;
        result.algorithm = "LSH";
        result.dataset = dataset_name;
        result.parameters["k"] = std::to_string(k);
        result.parameters["L"] = std::to_string(L);
        result.parameters["w"] = std::to_string(w);
        result.parameters["N"] = std::to_string(N);
        result.parameters["R"] = std::to_string(R);
        result.dataset_size = dataset.size();
        result.query_count = queries.size();
        result.k_neighbors = N;
        result.range_radius = R;
        
        std::cout << "Testing LSH with parameters: k=" << k << " L=" << L << " w=" << w 
                  << " N=" << N << " R=" << R << std::endl;
        
        // Initialize LSH with default seed = 1
        uint32_t M = 1000;  // Hash table size
        uint64_t m = 4294967291ULL;  // 2^32 - 5
        int seed = 1; // Default seed as per specification
        LSH<T> lsh(L, m, M, dataset.size(), k, dataset[0].size(), w, 
                   const_cast<std::vector<std::vector<T>>&>(dataset), seed);
        
        // Performance metrics
        double total_af = 0.0;
        double total_recall = 0.0;
        std::vector<double> query_times;
        
        size_t valid_queries = 0;
        
        // Test on subset of queries for faster evaluation
        size_t max_queries = std::min(static_cast<size_t>(100), queries.size());
        
        for (size_t i = 0; i < max_queries; ++i) {
            try {
                // Compute ground truth
                auto true_results = computeTrueKNN(queries[i], dataset, N);
                
                // Measure query time
                auto start = std::chrono::high_resolution_clock::now();
                auto approx_results = lsh.kNearestNeighbour(queries[i], N);
                auto end = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                query_times.push_back(duration.count() / 1000.0);  // Convert to milliseconds
                
                if (!approx_results.empty() && !true_results.empty()) {
                    total_af += computeApproximationFactor(approx_results, true_results);
                    total_recall += computeRecallAtN(approx_results, true_results);
                    valid_queries++;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing query " << i << ": " << e.what() << std::endl;
            }
        }
        
        // Calculate final metrics
        if (valid_queries > 0) {
            result.mean_approximation_factor = total_af / valid_queries;
            result.recall_at_n = total_recall / valid_queries;
        }
        
        if (!query_times.empty()) {
            result.mean_query_time_ms = std::accumulate(query_times.begin(), query_times.end(), 0.0) / query_times.size();
            result.queries_per_second = 1000.0 / result.mean_query_time_ms;
        }
        
        return result;
    }
    
    // Test Hypercube algorithm
    template<typename T>
    BenchmarkResult testHypercube(const std::vector<std::vector<T>>& dataset,
                                 const std::vector<std::vector<T>>& queries,
                                 const std::string& dataset_name,
                                 uint32_t kproj = 14, double w = 4.0, uint32_t M = 10,
                                 uint32_t probes = 2, uint32_t N = 5, double R = 2.0) {
        
        BenchmarkResult result;
        result.algorithm = "Hypercube";
        result.dataset = dataset_name;
        result.parameters["kproj"] = std::to_string(kproj);
        result.parameters["w"] = std::to_string(w);
        result.parameters["M"] = std::to_string(M);
        result.parameters["probes"] = std::to_string(probes);
        result.parameters["N"] = std::to_string(N);
        result.parameters["R"] = std::to_string(R);
        result.dataset_size = dataset.size();
        result.query_count = queries.size();
        result.k_neighbors = N;
        result.range_radius = R;
        
        std::cout << "Testing Hypercube with parameters: kproj=" << kproj << " w=" << w 
                  << " M=" << M << " probes=" << probes << " N=" << N << " R=" << R << std::endl;
        
        // Initialize Hypercube
        uint64_t m = 4294967291ULL;  // 2^32 - 5
        Hypercube<T> hypercube(kproj, m, M, probes, dataset.size(), w, dataset[0].size(),
                              const_cast<std::vector<std::vector<T>>&>(dataset), 1);
        
        // Performance metrics
        double total_af = 0.0;
        double total_recall = 0.0;
        std::vector<double> query_times;
        
        size_t valid_queries = 0;
        size_t max_queries = std::min(static_cast<size_t>(100), queries.size());
        
        for (size_t i = 0; i < max_queries; ++i) {
            try {
                // Compute ground truth
                auto true_results = computeTrueKNN(queries[i], dataset, N);
                
                // Measure query time
                auto start = std::chrono::high_resolution_clock::now();
                auto approx_results = hypercube.kNearestNeighbour(queries[i], N);
                auto end = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                query_times.push_back(duration.count() / 1000.0);
                
                if (!approx_results.empty() && !true_results.empty()) {
                    total_af += computeApproximationFactor(approx_results, true_results);
                    total_recall += computeRecallAtN(approx_results, true_results);
                    valid_queries++;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing query " << i << ": " << e.what() << std::endl;
            }
        }
        
        // Calculate final metrics
        if (valid_queries > 0) {
            result.mean_approximation_factor = total_af / valid_queries;
            result.recall_at_n = total_recall / valid_queries;
        }
        
        if (!query_times.empty()) {
            result.mean_query_time_ms = std::accumulate(query_times.begin(), query_times.end(), 0.0) / query_times.size();
            result.queries_per_second = 1000.0 / result.mean_query_time_ms;
        }
        
        return result;
    }
    
    // Test IVFFlat algorithm
    template<typename T>
    BenchmarkResult testIVFFlat(const std::vector<std::vector<T>>& dataset,
                               const std::vector<std::vector<T>>& queries,
                               const std::string& dataset_name,
                               uint32_t kclusters = 50, uint32_t nprobe = 5,
                               uint32_t N = 5, double R = 2.0) {
        
        BenchmarkResult result;
        result.algorithm = "IVFFlat";
        result.dataset = dataset_name;
        result.parameters["kclusters"] = std::to_string(kclusters);
        result.parameters["nprobe"] = std::to_string(nprobe);
        result.parameters["N"] = std::to_string(N);
        result.parameters["R"] = std::to_string(R);
        result.dataset_size = dataset.size();
        result.query_count = queries.size();
        result.k_neighbors = N;
        result.range_radius = R;
        
        std::cout << "Testing IVFFlat with parameters: kclusters=" << kclusters 
                  << " nprobe=" << nprobe << " N=" << N << " R=" << R << std::endl;
        
        try {
            // Initialize IVFFlat
            IVFFlat<T> ivfflat(kclusters, nprobe);
            ivfflat.train(dataset);
            ivfflat.add(dataset);
            
            // Performance metrics
            double total_af = 0.0;
            double total_recall = 0.0;
            std::vector<double> query_times;
            
            size_t valid_queries = 0;
            size_t max_queries = std::min(static_cast<size_t>(100), queries.size());
            
            for (size_t i = 0; i < max_queries; ++i) {
                try {
                    // Compute ground truth
                    auto true_results = computeTrueKNN(queries[i], dataset, N);
                    
                    // Measure query time
                    auto start = std::chrono::high_resolution_clock::now();
                    auto approx_results = ivfflat.search_k_nearest(queries[i], N);
                    auto end = std::chrono::high_resolution_clock::now();
                    
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                    query_times.push_back(duration.count() / 1000.0);
                    
                    if (!approx_results.empty() && !true_results.empty()) {
                        total_af += computeApproximationFactor(approx_results, true_results);
                        total_recall += computeRecallAtN(approx_results, true_results);
                        valid_queries++;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error processing query " << i << ": " << e.what() << std::endl;
                }
            }
            
            // Calculate final metrics
            if (valid_queries > 0) {
                result.mean_approximation_factor = total_af / valid_queries;
                result.recall_at_n = total_recall / valid_queries;
            }
            
            if (!query_times.empty()) {
                result.mean_query_time_ms = std::accumulate(query_times.begin(), query_times.end(), 0.0) / query_times.size();
                result.queries_per_second = 1000.0 / result.mean_query_time_ms;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error initializing IVFFlat: " << e.what() << std::endl;
        }
        
        return result;
    }
    
    // Test IVFPQ algorithm  
    template<typename T>
    BenchmarkResult testIVFPQ(const std::vector<std::vector<T>>& dataset,
                             const std::vector<std::vector<T>>& queries,
                             const std::string& dataset_name,
                             uint32_t kclusters = 50, uint32_t nprobe = 5,
                             uint32_t M = 16, uint32_t nbits = 8,
                             uint32_t N = 5, double R = 2.0) {
        
        BenchmarkResult result;
        result.algorithm = "IVFPQ";
        result.dataset = dataset_name;
        result.parameters["kclusters"] = std::to_string(kclusters);
        result.parameters["nprobe"] = std::to_string(nprobe);
        result.parameters["M"] = std::to_string(M);
        result.parameters["nbits"] = std::to_string(nbits);
        result.parameters["N"] = std::to_string(N);
        result.parameters["R"] = std::to_string(R);
        result.dataset_size = dataset.size();
        result.query_count = queries.size();
        result.k_neighbors = N;
        result.range_radius = R;
        
        std::cout << "Testing IVFPQ with parameters: kclusters=" << kclusters 
                  << " nprobe=" << nprobe << " M=" << M << " nbits=" << nbits 
                  << " N=" << N << " R=" << R << std::endl;
        
        try {
            // Initialize IVFPQ
            IVFPQ<T> ivfpq(kclusters, nprobe, M, nbits, dataset[0].size(), dataset);
            ivfpq.train();
            ivfpq.add_vectors(dataset);
            
            // Performance metrics
            double total_af = 0.0;
            double total_recall = 0.0;
            std::vector<double> query_times;
            
            size_t valid_queries = 0;
            size_t max_queries = std::min(static_cast<size_t>(100), queries.size());
            
            for (size_t i = 0; i < max_queries; ++i) {
                try {
                    // Compute ground truth
                    auto true_results = computeTrueKNN(queries[i], dataset, N);
                    
                    // Measure query time
                    auto start = std::chrono::high_resolution_clock::now();
                    auto approx_results = ivfpq.search_k_nearest(queries[i], N);
                    auto end = std::chrono::high_resolution_clock::now();
                    
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                    query_times.push_back(duration.count() / 1000.0);
                    
                    if (!approx_results.empty() && !true_results.empty()) {
                        total_af += computeApproximationFactor(approx_results, true_results);
                        total_recall += computeRecallAtN(approx_results, true_results);
                        valid_queries++;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error processing query " << i << ": " << e.what() << std::endl;
                }
            }
            
            // Calculate final metrics
            if (valid_queries > 0) {
                result.mean_approximation_factor = total_af / valid_queries;
                result.recall_at_n = total_recall / valid_queries;
            }
            
            if (!query_times.empty()) {
                result.mean_query_time_ms = std::accumulate(query_times.begin(), query_times.end(), 0.0) / query_times.size();
                result.queries_per_second = 1000.0 / result.mean_query_time_ms;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error initializing IVFPQ: " << e.what() << std::endl;
        }
        
        return result;
    }
    
    // Run comprehensive tests on all algorithms
    void runAllTests() {
        std::cout << "=== COMPREHENSIVE ALGORITHM COMPARISON STUDY ===" << std::endl;
        std::cout << "As required by PDFs: Approximation Factor, Recall@N, QPS Analysis" << std::endl;
        std::cout << std::endl;
        
        // Test SIFT dataset
        try {
            std::cout << "Loading SIFT dataset..." << std::endl;
            auto sift_data = SIFT::parseSIFTFile("dataset/sift_base.fvecs");
            auto sift_queries = SIFT::parseSIFTFile("dataset/sift_query.fvecs");
            
            // Limit dataset size for faster testing
            if (sift_data.size() > 10000) {
                sift_data.resize(10000);
                std::cout << "Limited SIFT dataset to 10000 vectors for performance" << std::endl;
            }
            if (sift_queries.size() > 100) {
                sift_queries.resize(100);
                std::cout << "Limited SIFT queries to 100 for performance" << std::endl;
            }
            
            std::cout << "Testing algorithms on SIFT dataset (" << sift_data.size() 
                      << " vectors, " << sift_queries.size() << " queries)" << std::endl;
            
            // Test different parameter configurations
            results.push_back(testLSH(sift_data, sift_queries, "SIFT", 4, 5, 4, 5, 2.0));
            results.push_back(testLSH(sift_data, sift_queries, "SIFT", 6, 8, 3, 10, 1.5));
            
            results.push_back(testHypercube(sift_data, sift_queries, "SIFT", 14, 4.0, 10, 2, 5, 2.0));
            results.push_back(testHypercube(sift_data, sift_queries, "SIFT", 16, 3.0, 15, 3, 10, 1.5));
            
            results.push_back(testIVFFlat(sift_data, sift_queries, "SIFT", 50, 5, 5, 2.0));
            results.push_back(testIVFFlat(sift_data, sift_queries, "SIFT", 100, 10, 10, 1.5));
            
            results.push_back(testIVFPQ(sift_data, sift_queries, "SIFT", 50, 5, 16, 8, 5, 2.0));
            results.push_back(testIVFPQ(sift_data, sift_queries, "SIFT", 100, 10, 32, 8, 10, 1.5));
            
        } catch (const std::exception& e) {
            std::cerr << "Error testing SIFT: " << e.what() << std::endl;
        }
        
        // Test MNIST dataset
        try {
            std::cout << "\nLoading MNIST dataset..." << std::endl;
            auto mnist_data = MNIST::parseMNISTImages("dataset/train-images.idx3-ubyte", 0, true);
            auto mnist_queries = MNIST::parseMNISTImages("dataset/t10k-images.idx3-ubyte", 0, true);
            
            // Limit dataset size for faster testing
            if (mnist_data.size() > 10000) {
                mnist_data.resize(10000);
                std::cout << "Limited MNIST dataset to 10000 images for performance" << std::endl;
            }
            if (mnist_queries.size() > 100) {
                mnist_queries.resize(100);
                std::cout << "Limited MNIST queries to 100 for performance" << std::endl;
            }
            
            std::cout << "Testing algorithms on MNIST dataset (" << mnist_data.size() 
                      << " images, " << mnist_queries.size() << " queries)" << std::endl;
            
            // Test with MNIST-optimized parameters
            results.push_back(testLSH(mnist_data, mnist_queries, "MNIST", 4, 5, 10, 5, 2000.0));
            results.push_back(testLSH(mnist_data, mnist_queries, "MNIST", 6, 8, 8, 10, 1500.0));
            
            results.push_back(testHypercube(mnist_data, mnist_queries, "MNIST", 14, 10.0, 10, 2, 5, 2000.0));
            results.push_back(testHypercube(mnist_data, mnist_queries, "MNIST", 16, 8.0, 15, 3, 10, 1500.0));
            
            results.push_back(testIVFFlat(mnist_data, mnist_queries, "MNIST", 50, 5, 5, 2000.0));
            results.push_back(testIVFFlat(mnist_data, mnist_queries, "MNIST", 100, 10, 10, 1500.0));
            
            results.push_back(testIVFPQ(mnist_data, mnist_queries, "MNIST", 50, 5, 16, 8, 5, 2000.0));
            results.push_back(testIVFPQ(mnist_data, mnist_queries, "MNIST", 100, 10, 32, 8, 10, 1500.0));
            
        } catch (const std::exception& e) {
            std::cerr << "Error testing MNIST: " << e.what() << std::endl;
        }
    }
    
    // Export results to CSV for further analysis
    void exportResults(const std::string& filename) {
        std::ofstream file(filename);
        
        // CSV Header
        file << "Algorithm,Dataset,";
        file << "MeanApproximationFactor,RecallAtN,QueriesPerSecond,MeanQueryTimeMS,";
        file << "DatasetSize,QueryCount,KNeighbors,RangeRadius,";
        file << "Parameters" << std::endl;
        
        for (const auto& result : results) {
            file << result.algorithm << "," << result.dataset << ",";
            file << std::fixed << std::setprecision(6);
            file << result.mean_approximation_factor << ",";
            file << result.recall_at_n << ",";
            file << result.queries_per_second << ",";
            file << result.mean_query_time_ms << ",";
            file << result.dataset_size << ",";
            file << result.query_count << ",";
            file << result.k_neighbors << ",";
            file << result.range_radius << ",";
            
            // Parameters as JSON-like string
            file << "\"";
            bool first = true;
            for (const auto& param : result.parameters) {
                if (!first) file << ";";
                file << param.first << "=" << param.second;
                first = false;
            }
            file << "\"" << std::endl;
        }
        
        std::cout << "Results exported to: " << filename << std::endl;
    }
    
    // Print summary report
    void printSummary() {
        if (results.empty()) {
            std::cout << "No results to display." << std::endl;
            return;
        }
        
        std::cout << "\n=== EXPERIMENTAL COMPARISON STUDY RESULTS ===" << std::endl;
        std::cout << "Based on requirements from PDFs: AF, Recall@N, QPS metrics" << std::endl;
        std::cout << std::endl;
        
        // Group results by dataset
        std::map<std::string, std::vector<BenchmarkResult*>> by_dataset;
        for (auto& result : results) {
            by_dataset[result.dataset].push_back(&result);
        }
        
        for (const auto& dataset_group : by_dataset) {
            std::cout << "=== " << dataset_group.first << " Dataset Results ===" << std::endl;
            std::cout << std::left;
            std::cout << std::setw(15) << "Algorithm";
            std::cout << std::setw(15) << "Approx.Factor";
            std::cout << std::setw(15) << "Recall@N";
            std::cout << std::setw(15) << "QPS";
            std::cout << std::setw(15) << "Query Time(ms)";
            std::cout << std::endl;
            std::cout << std::string(75, '-') << std::endl;
            
            for (const auto& result : dataset_group.second) {
                std::cout << std::setw(15) << result->algorithm;
                std::cout << std::setw(15) << std::fixed << std::setprecision(3) << result->mean_approximation_factor;
                std::cout << std::setw(15) << std::fixed << std::setprecision(3) << result->recall_at_n;
                std::cout << std::setw(15) << std::fixed << std::setprecision(1) << result->queries_per_second;
                std::cout << std::setw(15) << std::fixed << std::setprecision(2) << result->mean_query_time_ms;
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
        
        // Overall best performers
        std::cout << "=== BEST PERFORMERS ANALYSIS ===" << std::endl;
        
        // Best approximation factor
        auto best_af = std::min_element(results.begin(), results.end(),
            [](const BenchmarkResult& a, const BenchmarkResult& b) {
                return a.mean_approximation_factor < b.mean_approximation_factor && a.mean_approximation_factor > 0;
            });
        
        if (best_af != results.end() && best_af->mean_approximation_factor > 0) {
            std::cout << "Best Approximation Factor: " << best_af->algorithm 
                      << " on " << best_af->dataset 
                      << " (AF=" << std::fixed << std::setprecision(3) << best_af->mean_approximation_factor << ")" << std::endl;
        }
        
        // Best recall
        auto best_recall = std::max_element(results.begin(), results.end(),
            [](const BenchmarkResult& a, const BenchmarkResult& b) {
                return a.recall_at_n < b.recall_at_n;
            });
        
        if (best_recall != results.end()) {
            std::cout << "Best Recall@N: " << best_recall->algorithm 
                      << " on " << best_recall->dataset 
                      << " (Recall=" << std::fixed << std::setprecision(3) << best_recall->recall_at_n << ")" << std::endl;
        }
        
        // Best QPS
        auto best_qps = std::max_element(results.begin(), results.end(),
            [](const BenchmarkResult& a, const BenchmarkResult& b) {
                return a.queries_per_second < b.queries_per_second;
            });
        
        if (best_qps != results.end()) {
            std::cout << "Best Queries Per Second: " << best_qps->algorithm 
                      << " on " << best_qps->dataset 
                      << " (QPS=" << std::fixed << std::setprecision(1) << best_qps->queries_per_second << ")" << std::endl;
        }
        
        std::cout << std::endl;
    }
};

int main() {
    try {
        std::cout << "Starting Comprehensive Vector Similarity Search Benchmark" << std::endl;
        std::cout << "Implementing experimental study as specified in project PDFs" << std::endl;
        std::cout << "Measuring: Approximation Factor, Recall@N, Queries Per Second" << std::endl;
        std::cout << std::endl;
        
        ComprehensiveBenchmark benchmark;
        
        // Run all tests
        benchmark.runAllTests();
        
        // Print summary
        benchmark.printSummary();
        
        // Export results
        Utils::FileUtils::ensureResultsPath("comprehensive_benchmark_results.csv");
        benchmark.exportResults("results/comprehensive_benchmark_results.csv");
        
        std::cout << "\nBenchmark completed successfully!" << std::endl;
        std::cout << "Results saved to: results/comprehensive_benchmark_results.csv" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}