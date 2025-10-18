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
#include <filesystem>

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
#include "utils/visualization/visualizer.h"
#include "algorithms/compare/compareAlgo.h"

class VisualBenchmark {
private:
    std::vector<BenchmarkResult<float>> results;
    ResultsVisualizer<float> visualizer;
    
    // Ground truth computation using brute force
    std::vector<std::pair<int, float>> computeTrueKNN(const std::vector<float>& query, 
                                                      const std::vector<std::vector<float>>& dataset, 
                                                      int k) {
        std::vector<std::pair<int, float>> distances;
        
        for (size_t i = 0; i < dataset.size(); ++i) {
            float dist = metrics::EuclideanDistance(query, dataset[i], query.size());
            distances.emplace_back(static_cast<int>(i), dist);
        }
        
        std::sort(distances.begin(), distances.end(), 
                  [](const std::pair<int, float>& a, const std::pair<int, float>& b) {
                      return a.second < b.second;
                  });
        
        distances.resize(std::min(k, static_cast<int>(distances.size())));
        return distances;
    }
    
    // Compute approximation factor
    double computeApproximationFactor(const std::list<std::pair<int, float>>& approx_results,
                                     const std::vector<std::pair<int, float>>& true_results) {
        if (approx_results.empty() || true_results.empty()) return 0.0;
        
        double total_af = 0.0;
        size_t count = 0;
        
        auto it = approx_results.begin();
        for (size_t i = 0; i < true_results.size() && it != approx_results.end(); ++it, ++i) {
            if (true_results[i].second > 0) {
                total_af += it->second / true_results[i].second;
                count++;
            }
        }
        
        return count > 0 ? total_af / count : 0.0;
    }
    
    // Compute recall@N
    double computeRecallAtN(const std::list<std::pair<int, float>>& approx_results,
                           const std::vector<std::pair<int, float>>& true_results) {
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
    VisualBenchmark() : visualizer("results/visualization") {
        // Create results and visualization directories
        std::filesystem::create_directories("results");
        std::filesystem::create_directories("results/visualization");
    }
    
    // Test LSH algorithm with comprehensive parameter analysis
    BenchmarkResult<float> testLSH(const std::vector<std::vector<float>>& dataset,
                                   const std::vector<std::vector<float>>& queries,
                                   const std::string& dataset_name,
                                   uint32_t k = 4, uint32_t L = 5, uint32_t w = 4,
                                   uint32_t N = 5, double R = 2000.0) {
        
        BenchmarkResult<float> result;
        result.algorithm_name = "LSH";
        result.dataset_name = dataset_name;
        result.parameters["k"] = std::to_string(k);
        result.parameters["L"] = std::to_string(L);
        result.parameters["w"] = std::to_string(w);
        result.parameters["N"] = std::to_string(N);
        result.parameters["R"] = std::to_string(R);
        result.dataset_size = dataset.size();
        result.query_count = queries.size();
        
        std::cout << "Testing LSH with parameters: k=" << k << " L=" << L << " w=" << w 
                  << " N=" << N << " R=" << R << " on " << dataset_name << std::endl;
        
        try {
            // Initialize LSH
            uint32_t M = 1000;  // Hash table size
            uint64_t m = 4294967291ULL;  // 2^32 - 5
            LSH<float> lsh(L, m, M, dataset.size(), k, dataset[0].size(), w, 
                          const_cast<std::vector<std::vector<float>>&>(dataset));
            
            // Performance metrics
            double total_af = 0.0;
            double total_recall = 0.0;
            std::vector<double> query_times;
            
            size_t valid_queries = 0;
            size_t max_queries = std::min(static_cast<size_t>(50), queries.size());
            
            for (size_t i = 0; i < max_queries; ++i) {
                auto start = std::chrono::high_resolution_clock::now();
                auto approx_results = lsh.kNearestNeighbour(queries[i], N);
                auto end = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                query_times.push_back(duration.count() / 1000.0); // Convert to milliseconds
                
                if (!approx_results.empty()) {
                    auto true_results = computeTrueKNN(queries[i], dataset, N);
                    
                    double af = computeApproximationFactor(approx_results, true_results);
                    double recall = computeRecallAtN(approx_results, true_results);
                    
                    total_af += af;
                    total_recall += recall;
                    valid_queries++;
                }
            }
            
            // Calculate final metrics
            if (valid_queries > 0) {
                result.mean_approximation_factor = total_af / valid_queries;
                result.recall_at_n = total_recall / valid_queries;
            }
            
            if (!query_times.empty()) {
                result.mean_query_time = std::accumulate(query_times.begin(), query_times.end(), 0.0) / query_times.size();
                result.queries_per_second = 1000.0 / result.mean_query_time; // Convert ms to QPS
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error in LSH test: " << e.what() << std::endl;
            result.mean_approximation_factor = 0.0;
            result.recall_at_n = 0.0;
            result.queries_per_second = 0.0;
            result.mean_query_time = 0.0;
        }
        
        return result;
    }
    
    // Test Hypercube algorithm
    BenchmarkResult<float> testHypercube(const std::vector<std::vector<float>>& dataset,
                                         const std::vector<std::vector<float>>& queries,
                                         const std::string& dataset_name,
                                         uint32_t kproj = 14, double w = 4.0, uint32_t M = 10,
                                         uint32_t probes = 2, uint32_t N = 5, double R = 2000.0) {
        
        BenchmarkResult<float> result;
        result.algorithm_name = "Hypercube";
        result.dataset_name = dataset_name;
        result.parameters["kproj"] = std::to_string(kproj);
        result.parameters["w"] = std::to_string(w);
        result.parameters["M"] = std::to_string(M);
        result.parameters["probes"] = std::to_string(probes);
        result.parameters["N"] = std::to_string(N);
        result.parameters["R"] = std::to_string(R);
        result.dataset_size = dataset.size();
        result.query_count = queries.size();
        
        std::cout << "Testing Hypercube with parameters: kproj=" << kproj << " w=" << w 
                  << " M=" << M << " probes=" << probes << " N=" << N << " R=" << R 
                  << " on " << dataset_name << std::endl;
        
        try {
            // Initialize Hypercube
            uint64_t m = 4294967291ULL;  // 2^32 - 5
            Hypercube<float> hypercube(kproj, m, M, probes, dataset.size(), w, dataset[0].size(),
                                      const_cast<std::vector<std::vector<float>>&>(dataset), 1);
            
            // Performance metrics
            double total_af = 0.0;
            double total_recall = 0.0;
            std::vector<double> query_times;
            
            size_t valid_queries = 0;
            size_t max_queries = std::min(static_cast<size_t>(50), queries.size());
            
            for (size_t i = 0; i < max_queries; ++i) {
                auto start = std::chrono::high_resolution_clock::now();
                auto approx_results = hypercube.kNearestNeighbour(queries[i], N);
                auto end = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                query_times.push_back(duration.count() / 1000.0); // Convert to milliseconds
                
                if (!approx_results.empty()) {
                    auto true_results = computeTrueKNN(queries[i], dataset, N);
                    
                    double af = computeApproximationFactor(approx_results, true_results);
                    double recall = computeRecallAtN(approx_results, true_results);
                    
                    total_af += af;
                    total_recall += recall;
                    valid_queries++;
                }
            }
            
            // Calculate final metrics
            if (valid_queries > 0) {
                result.mean_approximation_factor = total_af / valid_queries;
                result.recall_at_n = total_recall / valid_queries;
            }
            
            if (!query_times.empty()) {
                result.mean_query_time = std::accumulate(query_times.begin(), query_times.end(), 0.0) / query_times.size();
                result.queries_per_second = 1000.0 / result.mean_query_time; // Convert ms to QPS
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error in Hypercube test: " << e.what() << std::endl;
            result.mean_approximation_factor = 0.0;
            result.recall_at_n = 0.0;
            result.queries_per_second = 0.0;
            result.mean_query_time = 0.0;
        }
        
        return result;
    }
    
    // Test IVFFlat algorithm
    BenchmarkResult<float> testIVFFlat(const std::vector<std::vector<float>>& dataset,
                                       const std::vector<std::vector<float>>& queries,
                                       const std::string& dataset_name,
                                       uint32_t kclusters = 50, uint32_t nprobe = 5,
                                       uint32_t N = 5, double R = 2000.0) {
        
        BenchmarkResult<float> result;
        result.algorithm_name = "IVFFlat";
        result.dataset_name = dataset_name;
        result.parameters["kclusters"] = std::to_string(kclusters);
        result.parameters["nprobe"] = std::to_string(nprobe);
        result.parameters["N"] = std::to_string(N);
        result.parameters["R"] = std::to_string(R);
        result.dataset_size = dataset.size();
        result.query_count = queries.size();
        
        std::cout << "Testing IVFFlat with parameters: kclusters=" << kclusters 
                  << " nprobe=" << nprobe << " N=" << N << " R=" << R 
                  << " on " << dataset_name << std::endl;
        
        try {
            // Initialize IVFFlat
            IVFFlat<float> ivfflat(kclusters, nprobe);
            
            // Train with dataset
            if (!ivfflat.train(dataset)) {
                std::cerr << "Failed to train IVFFlat" << std::endl;
                return result;
            }
            
            // Performance metrics
            double total_af = 0.0;
            double total_recall = 0.0;
            std::vector<double> query_times;
            
            size_t valid_queries = 0;
            size_t max_queries = std::min(static_cast<size_t>(50), queries.size());
            
            for (size_t i = 0; i < max_queries; ++i) {
                auto start = std::chrono::high_resolution_clock::now();
                auto approx_results = ivfflat.search_k_nearest(queries[i], N);
                auto end = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                query_times.push_back(duration.count() / 1000.0); // Convert to milliseconds
                
                if (!approx_results.empty()) {
                    auto true_results = computeTrueKNN(queries[i], dataset, N);
                    
                    double af = computeApproximationFactor(approx_results, true_results);
                    double recall = computeRecallAtN(approx_results, true_results);
                    
                    total_af += af;
                    total_recall += recall;
                    valid_queries++;
                }
            }
            
            // Calculate final metrics
            if (valid_queries > 0) {
                result.mean_approximation_factor = total_af / valid_queries;
                result.recall_at_n = total_recall / valid_queries;
            }
            
            if (!query_times.empty()) {
                result.mean_query_time = std::accumulate(query_times.begin(), query_times.end(), 0.0) / query_times.size();
                result.queries_per_second = 1000.0 / result.mean_query_time; // Convert ms to QPS
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error in IVFFlat test: " << e.what() << std::endl;
            result.mean_approximation_factor = 0.0;
            result.recall_at_n = 0.0;
            result.queries_per_second = 0.0;
            result.mean_query_time = 0.0;
        }
        
        return result;
    }
    
    // Test IVFPQ algorithm  
    BenchmarkResult<float> testIVFPQ(const std::vector<std::vector<float>>& dataset,
                                     const std::vector<std::vector<float>>& queries,
                                     const std::string& dataset_name,
                                     uint32_t kclusters = 50, uint32_t nprobe = 5,
                                     uint32_t M = 16, uint32_t nbits = 8,
                                     uint32_t N = 5, double R = 2000.0) {
        
        BenchmarkResult<float> result;
        result.algorithm_name = "IVFPQ";
        result.dataset_name = dataset_name;
        result.parameters["kclusters"] = std::to_string(kclusters);
        result.parameters["nprobe"] = std::to_string(nprobe);
        result.parameters["M"] = std::to_string(M);
        result.parameters["nbits"] = std::to_string(nbits);
        result.parameters["N"] = std::to_string(N);
        result.parameters["R"] = std::to_string(R);
        result.dataset_size = dataset.size();
        result.query_count = queries.size();
        
        std::cout << "Testing IVFPQ with parameters: kclusters=" << kclusters 
                  << " nprobe=" << nprobe << " M=" << M << " nbits=" << nbits 
                  << " N=" << N << " R=" << R << " on " << dataset_name << std::endl;
        
        try {
            // Initialize IVFPQ
            IVFPQ<float> ivfpq(kclusters, nprobe, M, nbits, dataset[0].size(), dataset);
            
            // Train the index
            ivfpq.train();
            
            // Performance metrics
            double total_af = 0.0;
            double total_recall = 0.0;
            std::vector<double> query_times;
            
            size_t valid_queries = 0;
            size_t max_queries = std::min(static_cast<size_t>(50), queries.size());
            
            for (size_t i = 0; i < max_queries; ++i) {
                auto start = std::chrono::high_resolution_clock::now();
                auto approx_results = ivfpq.search_k_nearest(queries[i], N);
                auto end = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                query_times.push_back(duration.count() / 1000.0); // Convert to milliseconds
                
                if (!approx_results.empty()) {
                    auto true_results = computeTrueKNN(queries[i], dataset, N);
                    
                    double af = computeApproximationFactor(approx_results, true_results);
                    double recall = computeRecallAtN(approx_results, true_results);
                    
                    total_af += af;
                    total_recall += recall;
                    valid_queries++;
                }
            }
            
            // Calculate final metrics
            if (valid_queries > 0) {
                result.mean_approximation_factor = total_af / valid_queries;
                result.recall_at_n = total_recall / valid_queries;
            }
            
            if (!query_times.empty()) {
                result.mean_query_time = std::accumulate(query_times.begin(), query_times.end(), 0.0) / query_times.size();
                result.queries_per_second = 1000.0 / result.mean_query_time; // Convert ms to QPS
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error in IVFPQ test: " << e.what() << std::endl;
            result.mean_approximation_factor = 0.0;
            result.recall_at_n = 0.0;
            result.queries_per_second = 0.0;
            result.mean_query_time = 0.0;
        }
        
        return result;
    }
    
    // Run comprehensive tests with parameter sweeps and visualization
    void runComprehensiveAnalysis() {
        std::cout << "=== COMPREHENSIVE VISUAL BENCHMARK ANALYSIS ===" << std::endl;
        std::cout << "Testing all algorithms with parameter sweeps and generating visualizations" << std::endl;
        std::cout << "Metrics: Approximation Factor, Recall@N, QPS, Query Time" << std::endl;
        std::cout << std::endl;
        
        // Test SIFT dataset
        try {
            std::cout << "Loading SIFT dataset..." << std::endl;
            auto sift_data = SIFT::parseSIFTFile("dataset/sift_base.fvecs");
            auto sift_queries = SIFT::parseSIFTFile("dataset/sift_query.fvecs");
            
            // Limit dataset size for comprehensive testing
            if (sift_data.size() > 5000) {
                sift_data.resize(5000);
                std::cout << "Limited SIFT dataset to 5000 vectors for comprehensive analysis" << std::endl;
            }
            if (sift_queries.size() > 50) {
                sift_queries.resize(50);
                std::cout << "Limited SIFT queries to 50 for comprehensive analysis" << std::endl;
            }
            
            std::cout << "Testing algorithms on SIFT dataset (" << sift_data.size() 
                      << " vectors, " << sift_queries.size() << " queries)" << std::endl;
            
            // LSH Parameter Sweep
            std::cout << "\n--- LSH Parameter Sweep on SIFT ---" << std::endl;
            std::vector<uint32_t> k_values = {4, 6, 8};
            std::vector<uint32_t> L_values = {5, 8, 12};
            std::vector<uint32_t> w_values = {3, 4, 5};
            
            for (uint32_t k : k_values) {
                for (uint32_t L : L_values) {
                    for (uint32_t w : w_values) {
                        results.push_back(testLSH(sift_data, sift_queries, "SIFT", k, L, w, 5, 2000.0));
                    }
                }
            }
            
            // Hypercube Parameter Sweep
            std::cout << "\n--- Hypercube Parameter Sweep on SIFT ---" << std::endl;
            std::vector<uint32_t> kproj_values = {12, 14, 16};
            std::vector<double> w_cube_values = {3.0, 4.0, 5.0};
            std::vector<uint32_t> M_values = {10, 15, 20};
            
            for (uint32_t kproj : kproj_values) {
                for (double w : w_cube_values) {
                    for (uint32_t M : M_values) {
                        results.push_back(testHypercube(sift_data, sift_queries, "SIFT", kproj, w, M, 2, 5, 2000.0));
                    }
                }
            }
            
            // IVFFlat Parameter Sweep - DISABLED
            std::cout << "\n--- IVFFlat Parameter Sweep on SIFT (SKIPPED) ---" << std::endl;
            std::cout << "IVFFlat testing disabled per user request" << std::endl;
            /*
            std::vector<uint32_t> clusters_values = {50, 100, 200};
            std::vector<uint32_t> nprobe_values = {5, 10, 15};
            
            for (uint32_t clusters : clusters_values) {
                for (uint32_t nprobe : nprobe_values) {
                    results.push_back(testIVFFlat(sift_data, sift_queries, "SIFT", clusters, nprobe, 5, 2000.0));
                }
            }
            */
            
            // IVFPQ Parameter Sweep - DISABLED
            std::cout << "\n--- IVFPQ Parameter Sweep on SIFT (SKIPPED) ---" << std::endl;
            std::cout << "IVFPQ testing disabled per user request" << std::endl;
            /*
            std::vector<uint32_t> M_pq_values = {8, 16, 32};
            std::vector<uint32_t> nbits_values = {6, 8};
            
            for (uint32_t clusters : {50, 100}) {
                for (uint32_t nprobe : {5, 10}) {
                    for (uint32_t M : M_pq_values) {
                        for (uint32_t nbits : nbits_values) {
                            results.push_back(testIVFPQ(sift_data, sift_queries, "SIFT", clusters, nprobe, M, nbits, 5, 2000.0));
                        }
                    }
                }
            }
            */
            
        } catch (const std::exception& e) {
            std::cerr << "Error testing SIFT: " << e.what() << std::endl;
        }
        
        // Test MNIST dataset with reduced parameter sweep
        try {
            std::cout << "\nLoading MNIST dataset..." << std::endl;
            auto mnist_data = MNIST::parseMNISTImages("dataset/train-images.idx3-ubyte", 0, true);
            auto mnist_queries = MNIST::parseMNISTImages("dataset/t10k-images.idx3-ubyte", 0, true);
            
            // Limit dataset size for faster testing
            if (mnist_data.size() > 3000) {
                mnist_data.resize(3000);
                std::cout << "Limited MNIST dataset to 3000 images for analysis" << std::endl;
            }
            if (mnist_queries.size() > 30) {
                mnist_queries.resize(30);
                std::cout << "Limited MNIST queries to 30 for analysis" << std::endl;
            }
            
            std::cout << "Testing algorithms on MNIST dataset (" << mnist_data.size() 
                      << " images, " << mnist_queries.size() << " queries)" << std::endl;
            
            // MNIST Parameter Tests (optimized for MNIST characteristics) - Only LSH and Hypercube
            std::cout << "\n--- Algorithm Tests on MNIST (LSH and Hypercube only) ---" << std::endl;
            results.push_back(testLSH(mnist_data, mnist_queries, "MNIST", 4, 5, 10, 5, 2.0));
            results.push_back(testLSH(mnist_data, mnist_queries, "MNIST", 6, 8, 15, 5, 2.0));
            results.push_back(testLSH(mnist_data, mnist_queries, "MNIST", 8, 10, 20, 5, 2.0));
            
            results.push_back(testHypercube(mnist_data, mnist_queries, "MNIST", 14, 10.0, 10, 2, 5, 2.0));
            results.push_back(testHypercube(mnist_data, mnist_queries, "MNIST", 16, 15.0, 15, 3, 5, 2.0));
            
            // IVFFlat and IVFPQ testing disabled per user request
            std::cout << "IVFFlat and IVFPQ testing skipped per user request" << std::endl;
            /*
            results.push_back(testIVFFlat(mnist_data, mnist_queries, "MNIST", 50, 5, 5, 2.0));
            results.push_back(testIVFFlat(mnist_data, mnist_queries, "MNIST", 100, 10, 5, 2.0));
            
            results.push_back(testIVFPQ(mnist_data, mnist_queries, "MNIST", 50, 5, 16, 8, 5, 2.0));
            results.push_back(testIVFPQ(mnist_data, mnist_queries, "MNIST", 100, 10, 32, 8, 5, 2.0));
            */
            
        } catch (const std::exception& e) {
            std::cerr << "Error testing MNIST: " << e.what() << std::endl;
        }
    }
    
    // Generate comprehensive visualizations and analysis
    void generateVisualizationReport() {
        if (results.empty()) {
            std::cerr << "No benchmark results available for visualization" << std::endl;
            return;
        }
        
        std::cout << "\n=== GENERATING COMPREHENSIVE VISUALIZATION REPORT ===" << std::endl;
        
        // Set results in visualizer
        visualizer.set_results(results);
        
        // Generate all comparison plots
        std::cout << "Generating algorithm comparison plots..." << std::endl;
        visualizer.plot_algorithm_comparison_bar("AF");
        visualizer.plot_algorithm_comparison_bar("Recall");
        visualizer.plot_algorithm_comparison_bar("QPS");
        visualizer.plot_algorithm_comparison_bar("Time");
        
        // Generate trade-off analysis
        std::cout << "Generating recall vs approximation factor analysis..." << std::endl;
        visualizer.plot_recall_vs_approximation_factor();
        
        // Generate parameter impact plots for each algorithm (LSH and Hypercube only)
        std::cout << "Generating parameter impact analysis..." << std::endl;
        
        // LSH parameter impact
        visualizer.plot_parameter_impact("LSH", "k", "AF");
        visualizer.plot_parameter_impact("LSH", "k", "Recall");
        visualizer.plot_parameter_impact("LSH", "L", "QPS");
        visualizer.plot_parameter_impact("LSH", "w", "Time");
        
        // Hypercube parameter impact
        visualizer.plot_parameter_impact("Hypercube", "kproj", "AF");
        visualizer.plot_parameter_impact("Hypercube", "kproj", "Recall");
        visualizer.plot_parameter_impact("Hypercube", "M", "QPS");
        visualizer.plot_parameter_impact("Hypercube", "w", "Time");
        
        // IVFFlat and IVFPQ parameter impact analysis disabled
        std::cout << "IVFFlat and IVFPQ parameter analysis skipped per user request" << std::endl;
        /*
        // IVFFlat parameter impact
        visualizer.plot_parameter_impact("IVFFlat", "kclusters", "AF");
        visualizer.plot_parameter_impact("IVFFlat", "nprobe", "Recall");
        visualizer.plot_parameter_impact("IVFFlat", "kclusters", "QPS");
        
        // IVFPQ parameter impact
        visualizer.plot_parameter_impact("IVFPQ", "M", "AF");
        visualizer.plot_parameter_impact("IVFPQ", "nbits", "Recall");
        visualizer.plot_parameter_impact("IVFPQ", "kclusters", "QPS");
        */
        
        // Generate comprehensive HTML report
        std::cout << "Generating HTML report..." << std::endl;
        visualizer.create_html_report("results/visualization/comprehensive_report.html");
        
        std::cout << "Visualization report generated successfully!" << std::endl;
        std::cout << "Check results/visualization/ directory for all plots and HTML report" << std::endl;
    }
    
    // Export results to CSV
    void exportResults(const std::string& filename) {
        std::string results_file = Utils::FileUtils::ensureResultsPath(filename);
        std::ofstream file(results_file);
        
        // CSV Header
        file << "Algorithm,Dataset,";
        file << "MeanApproximationFactor,RecallAtN,QueriesPerSecond,MeanQueryTimeMS,";
        file << "DatasetSize,QueryCount,";
        file << "Parameters" << std::endl;
        
        for (const auto& result : results) {
            file << result.algorithm_name << "," << result.dataset_name << ",";
            file << std::fixed << std::setprecision(6) << result.mean_approximation_factor << ",";
            file << std::fixed << std::setprecision(6) << result.recall_at_n << ",";
            file << std::fixed << std::setprecision(2) << result.queries_per_second << ",";
            file << std::fixed << std::setprecision(3) << result.mean_query_time << ",";
            file << result.dataset_size << "," << result.query_count << ",";
            
            // Parameters as JSON-like string
            file << "\"";
            for (auto it = result.parameters.begin(); it != result.parameters.end(); ++it) {
                if (it != result.parameters.begin()) file << ";";
                file << it->first << "=" << it->second;
            }
            file << "\"" << std::endl;
        }
        
        std::cout << "Results exported to: " << results_file << std::endl;
    }
    
    // Print summary report
    void printSummary() {
        if (results.empty()) {
            std::cout << "No benchmark results available" << std::endl;
            return;
        }
        
        std::cout << "\n=== COMPREHENSIVE BENCHMARK SUMMARY ===" << std::endl;
        std::cout << "Total algorithm configurations tested: " << results.size() << std::endl;
        std::cout << std::endl;
        
        // Group results by dataset
        std::map<std::string, std::vector<BenchmarkResult<float>*>> by_dataset;
        for (auto& result : results) {
            by_dataset[result.dataset_name].push_back(&result);
        }
        
        for (const auto& dataset_group : by_dataset) {
            std::cout << "Dataset: " << dataset_group.first << std::endl;
            std::cout << "Configurations tested: " << dataset_group.second.size() << std::endl;
            
            // Find best performers for this dataset
            auto best_af = std::min_element(dataset_group.second.begin(), dataset_group.second.end(),
                [](const BenchmarkResult<float>* a, const BenchmarkResult<float>* b) {
                    return a->mean_approximation_factor < b->mean_approximation_factor && a->mean_approximation_factor > 0;
                });
            
            auto best_recall = std::max_element(dataset_group.second.begin(), dataset_group.second.end(),
                [](const BenchmarkResult<float>* a, const BenchmarkResult<float>* b) {
                    return a->recall_at_n < b->recall_at_n;
                });
            
            auto best_qps = std::max_element(dataset_group.second.begin(), dataset_group.second.end(),
                [](const BenchmarkResult<float>* a, const BenchmarkResult<float>* b) {
                    return a->queries_per_second < b->queries_per_second;
                });
            
            if (best_af != dataset_group.second.end()) {
                std::cout << "  Best Approximation Factor: " << (*best_af)->algorithm_name 
                          << " (AF=" << (*best_af)->mean_approximation_factor << ")" << std::endl;
            }
            
            if (best_recall != dataset_group.second.end()) {
                std::cout << "  Best Recall@N: " << (*best_recall)->algorithm_name 
                          << " (Recall=" << (*best_recall)->recall_at_n << ")" << std::endl;
            }
            
            if (best_qps != dataset_group.second.end()) {
                std::cout << "  Best QPS: " << (*best_qps)->algorithm_name 
                          << " (QPS=" << (*best_qps)->queries_per_second << ")" << std::endl;
            }
            
            std::cout << std::endl;
        }
    }
};

int main() {
    try {
        std::cout << "Starting Comprehensive Visual Benchmark Analysis" << std::endl;
        std::cout << "Testing all algorithms with parameter sweeps and generating visualizations" << std::endl;
        std::cout << "This will create comprehensive performance analysis with plots and reports" << std::endl;
        std::cout << std::endl;
        
        VisualBenchmark benchmark;
        
        // Run comprehensive tests with parameter sweeps
        benchmark.runComprehensiveAnalysis();
        
        // Print summary
        benchmark.printSummary();
        
        // Export results to CSV
        benchmark.exportResults("visual_benchmark_results.csv");
        
        // Generate comprehensive visualization report
        benchmark.generateVisualizationReport();
        
        std::cout << "\n=== BENCHMARK COMPLETED SUCCESSFULLY ===" << std::endl;
        std::cout << "Results saved to:" << std::endl;
        std::cout << "  - results/visual_benchmark_results.csv (raw data)" << std::endl;
        std::cout << "  - results/visualization/ (all plots and analysis)" << std::endl;
        std::cout << "  - results/visualization/comprehensive_report.html (main report)" << std::endl;
        std::cout << std::endl;
        std::cout << "Open the HTML report in a web browser for interactive analysis!" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}