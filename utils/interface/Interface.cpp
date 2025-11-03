#include "Interface.h"
#include "../helpers/Utils.h"
#include "../math/math.h"
#include "../../dataprocessing/SIFT/SIFT.h"
#include "../../dataprocessing/MNIST/MNIST.h"
#include "../../algorithms/LSH/LSH.h"
#include "../../algorithms/hypercube/Hypercube.h"
#include "../../algorithms/IVFFlat/IVFFlat.h"
#include "../../algorithms/IVFPQ/IVFPQ.h"
#include "../../metrics/metrics.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <set>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <unordered_set>

Interface::Interface() {
}

Interface::~Interface() {
}

int Interface::runSearch(int argc, char* argv[]) {
    std::string input_file, query_file, output_file, type, method;
    int k = 4, L = 5, kproj = 14, kclusters = 50, nprobe = 5; // Default: kclusters=50, nprobe=5
    int M = 500, probes = 10, nbits = 8, N = 1, seed = 1; // Default: M=500 (max points), probes=10 (max buckets), seed=1
    int qmax = 0; // 0 means process all queries
    double w = 4.0, R = 10000.0; // Default w = 4.0
    bool range = false;
    bool auto_w = false; // When true, compute data-driven w and override CLI
    
    try {
    parseSearchArgs(argc, argv, input_file, query_file, output_file, type, method,
               k, L, kproj, kclusters, nprobe, M, probes, nbits, N, seed, w, R, range, qmax, auto_w);
        
        // Set default R values based on dataset type
        // Default: R = 2000 for MNIST, R = 2 for SIFT (as per specification)
        if (R == 10000.0) { // If R wasn't explicitly set
            if (type == "mnist") {
                R = 2000.0;
            } else if (type == "sift") {
                R = 2.0;
            }
        }
        
        // Execute the appropriate algorithm
        if (method == "lsh") {
            return executeLSH(input_file, query_file, output_file, type, k, L, w, N, R, range, qmax, auto_w);
        }
        else if (method == "hypercube") {
            return executeHypercube(input_file, query_file, output_file, type, kproj, w, M, probes, N, R, range, seed, qmax, auto_w);
        }
        else if (method == "ivfflat") {
            return executeIVFFlat(input_file, query_file, output_file, type, kclusters, nprobe, N, R, range, seed, qmax);
        }
        else if (method == "ivfpq") {
            return executeIVFPQ(input_file, query_file, output_file, type, kclusters, nprobe, M, nbits, N, R, range, seed, qmax);
        }
        else {
            std::cerr << "Error: Unknown method. Use -lsh, -hypercube, -ivfflat, or -ivfpq" << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

void Interface::parseSearchArgs(int argc, char* argv[], std::string& input_file, 
                               std::string& query_file, std::string& output_file,
                               std::string& type, std::string& method,
                               int& k, int& L, int& kproj, int& kclusters, int& nprobe,
                               int& M, int& probes, int& nbits, int& N, int& seed,
                               double& w, double& R, bool& range, int& qmax, bool& auto_w) {
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            input_file = argv[++i];
        }
        else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc) {
            query_file = argv[++i];
        }
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        }
        else if (strcmp(argv[i], "-type") == 0 && i + 1 < argc) {
            type = argv[++i];
        }
        else if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
            k = std::atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-L") == 0 && i + 1 < argc) {
            L = std::atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-kproj") == 0 && i + 1 < argc) {
            kproj = std::atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-kclusters") == 0 && i + 1 < argc) {
            kclusters = std::atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-nprobe") == 0 && i + 1 < argc) {
            nprobe = std::atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-M") == 0 && i + 1 < argc) {
            M = std::atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-probes") == 0 && i + 1 < argc) {
            probes = std::atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-nbits") == 0 && i + 1 < argc) {
            nbits = std::atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-N") == 0 && i + 1 < argc) {
            N = std::atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-seed") == 0 && i + 1 < argc) {
            seed = std::atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            w = std::atof(argv[++i]);
        }
        else if (strcmp(argv[i], "-R") == 0 && i + 1 < argc) {
            R = std::atof(argv[++i]);
        }
        else if (strcmp(argv[i], "-range") == 0 && i + 1 < argc) {
            std::string range_str = argv[++i];
            range = (range_str == "true");
        }
        else if (strcmp(argv[i], "-qmax") == 0 && i + 1 < argc) {
            qmax = std::atoi(argv[++i]);
            if (qmax < 0) qmax = 0;
        }
        else if (strcmp(argv[i], "-auto_w") == 0 && i + 1 < argc) {
            std::string v = argv[++i];
            auto_w = (v == "true" || v == "1" || v == "yes");
        }
        else if (strcmp(argv[i], "-lsh") == 0) {
            method = "lsh";
        }
        else if (strcmp(argv[i], "-hypercube") == 0) {
            method = "hypercube";
        }
        else if (strcmp(argv[i], "-ivfflat") == 0) {
            method = "ivfflat";
        }
        else if (strcmp(argv[i], "-ivfpq") == 0) {
            method = "ivfpq";
        }
    }
    
    // Validate required arguments
    if (input_file.empty() || query_file.empty() || output_file.empty() || 
        type.empty() || method.empty()) {
        throw std::runtime_error("Missing required arguments");
    }
    
    if (type != "sift" && type != "mnist") {
        throw std::runtime_error("Type must be 'sift' or 'mnist'");
    }
}

std::vector<std::vector<float>> Interface::parseDataset(const std::string& filename, const std::string& type) {
    if (type == "mnist") {
        return DataParser::parseMNIST(filename);
    }
    else if (type == "sift") {
        return SIFT::parseSIFTFile(filename);
    }
    else {
        throw std::runtime_error("Unknown data type: " + type);
    }
}

std::vector<std::vector<float>> Interface::parseQueryset(const std::string& filename, const std::string& type) {
    return parseDataset(filename, type);
}

int Interface::executeLSH(const std::string& input_file, const std::string& query_file,
                         const std::string& output_file, const std::string& type,
                         int k, int L, double w, int N, double R, bool range, int qmax, bool auto_w) {
    
    std::cout << "Executing LSH algorithm..." << std::endl;
    std::cout << "Parameters: k=" << k << ", L=" << L << ", w=" << w 
              << ", N=" << N << ", R=" << R << ", seed=1 (default)" << std::endl;
    
    // Parse datasets
    auto dataset = parseDataset(input_file, type);
    auto queries = parseQueryset(query_file, type);
    
    if (dataset.empty() || queries.empty()) {
        std::cerr << "Error: Failed to parse datasets." << std::endl;
        return 1;
    }
    
    std::cout << "Dataset size: " << dataset.size() << " vectors, " 
              << dataset[0].size() << " dimensions" << std::endl;
    std::cout << "Query set size: " << queries.size() << " vectors" << std::endl;
    
    // Determine w parameter
    double used_w = w;
    if (auto_w) {
        double computed_w = 20.0 * our_math::compute_w_value(dataset, 1000);
        used_w = computed_w;
        std::cout << "Computed w parameter: " << computed_w << " (overriding CLI w due to -auto_w)" << std::endl;
    } else {
        std::cout << "Using CLI w parameter: " << w << std::endl;
    }
    
    // Initialize LSH with default seed = 1
    uint32_t M = 1000007; // Large prime for hash table size
    uint64_t m = 4294967291ULL; // 2^32 - 5
    int seed = 1; // Default seed as per specification
    LSH<float> lsh(L, m, M, dataset.size(), k, dataset[0].size(), static_cast<uint32_t>(used_w), dataset, seed);
    
    // Ensure output file goes to results directory
    std::string results_output_file = Utils::FileUtils::ensureResultsPath(output_file);
    std::ofstream output(results_output_file);
    if (!output.is_open()) {
        std::cerr << "Error: Cannot create output file." << std::endl;
        return 1;
    }
    
    // Print method name
    output << "LSH" << std::endl;
    
    Utils::AutoTimer total_timer;
    total_timer.start();
    double total_approx_time = 0.0;
    double total_true_time = 0.0;
    double total_af = 0.0;
    double total_recall = 0.0;
    int valid_queries = 0;
    int valid_af_queries = 0;  // Separate counter for AF (excludes true_dist == 0)
    
    const size_t queries_to_process = qmax > 0 ? std::min(queries.size(), static_cast<size_t>(qmax)) : queries.size();
    for (size_t query_idx = 0; query_idx < queries_to_process; ++query_idx) {
        const auto& query = queries[query_idx];
        
        // Approximate search using LSH
        Utils::AutoTimer approx_timer;
        approx_timer.start();
        std::list<std::pair<int, float>> neighbors;
        
        if (N == 1) {
            auto result = lsh.NearestNeighbour(query);
            if (result.first != -1) {
                // For SIFT, skip zero distances
                if (type != "sift" || result.second != 0.0f) {
                    neighbors.push_back(result);
                }
            }
        } else {
            neighbors = lsh.kNearestNeighbour(query, N);
            // For SIFT, filter out zero distances
            if (type == "sift") {
                neighbors.remove_if([](const std::pair<int, float>& p) {
                    return p.second == 0.0f;
                });
            }
        }
        total_approx_time += approx_timer.elapsed();
        
        // True (brute force) search
        Utils::AutoTimer true_timer;
        true_timer.start();
        std::vector<std::pair<float, int>> all_distances;
        all_distances.reserve(dataset.size());
        
        for (size_t i = 0; i < dataset.size(); ++i) {
            float distance = metrics::EuclideanDistance(query, dataset[i], static_cast<int>(query.size()));
            // For SIFT dataset, skip zero distances (self-matches)
            if (type == "sift" && distance == 0.0f) {
                continue;
            }
            all_distances.emplace_back(distance, static_cast<int>(i));
        }
        
        std::partial_sort(all_distances.begin(), 
                         all_distances.begin() + std::min(N, static_cast<int>(all_distances.size())),
                         all_distances.end());
        
        std::list<std::pair<int, float>> true_neighbors;
        for (int i = 0; i < N && i < static_cast<int>(all_distances.size()); ++i) {
            true_neighbors.emplace_back(all_distances[i].second, all_distances[i].first);
        }
        total_true_time += true_timer.elapsed();
        
        // Calculate AF (Approximation Factor) for this query
        // AF = distanceApproximate / distanceTrue for the first neighbor
        double query_af = 0.0;
        double query_recall = 0.0;
        
        if (!neighbors.empty() && !true_neighbors.empty()) {
            valid_queries++;
            
            float approx_dist = neighbors.front().second;
            float true_dist = true_neighbors.front().second;
            
            // Only calculate AF if true_dist > 0 (exclude 0-denominator cases)
            if (true_dist > 0) {
                query_af = approx_dist / true_dist;
                total_af += query_af;
                valid_af_queries++;
            }
            
            // Calculate Recall@N for this query
            // Recall@N = (number of true neighbors found in approximate results) / N
            std::unordered_set<int> approx_ids;
            for (const auto& neighbor : neighbors) {
                approx_ids.insert(neighbor.first);
            }
            
            int matches = 0;
            for (const auto& true_neighbor : true_neighbors) {
                if (approx_ids.count(true_neighbor.first)) {
                    matches++;
                }
            }
            query_recall = static_cast<double>(matches) / N;
            total_recall += query_recall;
        }
        
        // Range search if requested
        std::list<int> range_result_ids;
        if (range) {
            auto range_results = lsh.RangeSearch(query, R, 1);
            for (const auto& result : range_results) {
                range_result_ids.push_back(result.first);
            }
        }
        
        // Use helper function to print results
        printSearchResults(output, "LSH", query_idx, neighbors, true_neighbors, range_result_ids, N, type, query, dataset);
    }
    
    double total_time_seconds = total_timer.elapsed();
    
    // Calculate average metrics
    double avg_af = valid_af_queries > 0 ? total_af / valid_af_queries : 0.0;
    double recall_at_n = valid_queries > 0 ? total_recall / valid_queries : 0.0;
    double avg_approx_time = queries_to_process > 0 ? total_approx_time / queries_to_process : 0.0;
    double avg_true_time = queries_to_process > 0 ? total_true_time / queries_to_process : 0.0;
    double qps = (total_time_seconds > 0) ? (static_cast<double>(queries_to_process) / total_time_seconds) : 0.0;
    
    // Use helper function to print final statistics
    printFinalStatistics(output, avg_af, recall_at_n, qps, avg_approx_time, avg_true_time);
    
    output.close();
    std::cout << "LSH search completed successfully" << std::endl;
    std::cout << "Results written to: " << results_output_file << std::endl;
    return 0;
}

int Interface::executeHypercube(const std::string& input_file, const std::string& query_file,
                               const std::string& output_file, const std::string& type,
                               int kproj, double w, int M, int probes, int N, double R, bool range, int seed, int qmax, bool auto_w) {
    
    std::cout << "Executing Hypercube algorithm..." << std::endl;
    std::cout << "Parameters: kproj=" << kproj << ", w=" << w << ", M=" << M 
              << ", probes=" << probes << ", N=" << N << ", R=" << R << ", seed=" << seed << std::endl;
    
    // Parse datasets
    auto dataset = parseDataset(input_file, type);
    auto queries = parseQueryset(query_file, type);
    
    if (dataset.empty() || queries.empty()) {
        std::cerr << "Error: Failed to parse datasets." << std::endl;
        return 1;
    }
    
    std::cout << "Loaded " << dataset.size() << " " << type << " vectors of dimension " 
              << dataset[0].size() << " from " << input_file << std::endl;
    std::cout << "Loaded " << queries.size() << " " << type << " vectors of dimension " 
              << queries[0].size() << " from " << query_file << std::endl;
    
    std::cout << "Dataset size: " << dataset.size() << " vectors, " 
              << dataset[0].size() << " dimensions" << std::endl;
    std::cout << "Query set size: " << queries.size() << " vectors" << std::endl;
    
    // Determine w parameter
    double used_w_hc = w;
    if (auto_w) {
        double computed_w = 20.0 * our_math::compute_w_value(dataset, 1000);
        used_w_hc = computed_w;
        std::cout << "Computed w parameter: " << computed_w << " (overriding CLI w due to -auto_w)" << std::endl;
    } else {
        std::cout << "Using CLI w parameter: " << w << std::endl;
    }
    
    // Initialize Hypercube with specified seed parameter
    uint64_t m = 4294967291ULL; // 2^32 - 5
    Hypercube<float> hypercube(kproj, m, M, probes, dataset.size(), used_w_hc, dataset[0].size(), dataset, seed);
    
    // Ensure output file goes to results directory
    std::string results_output_file = Utils::FileUtils::ensureResultsPath(output_file);
    std::ofstream output(results_output_file);
    if (!output.is_open()) {
        std::cerr << "Error: Cannot create output file." << std::endl;
        return 1;
    }
    
    // Print method name
    output << "Hypercube" << std::endl;
    
    Utils::AutoTimer total_timer;
    total_timer.start();
    double total_approx_time = 0.0;
    double total_true_time = 0.0;
    
    int successful_queries = 0;
    int valid_af_queries = 0;  // Separate counter for AF (excludes true_dist == 0)
    double total_af = 0.0;
    double total_recall = 0.0;
    
    // Process queries (limit for testing)
    size_t max_queries = qmax > 0 ? std::min(queries.size(), static_cast<size_t>(qmax)) : queries.size();
    for (size_t query_idx = 0; query_idx < max_queries; ++query_idx) {
        try {
            // Approximate search using Hypercube
            Utils::AutoTimer approx_timer;
            approx_timer.start();
            auto neighbors = hypercube.kNearestNeighbour(queries[query_idx], N);
            // For SIFT, filter out zero distances
            if (type == "sift") {
                neighbors.remove_if([](const std::pair<int, float>& p) {
                    return p.second == 0.0f;
                });
            }
            total_approx_time += approx_timer.elapsed();
            
            // True nearest neighbors (brute force)
            Utils::AutoTimer true_timer;
            true_timer.start();
            std::vector<std::pair<float, int>> all_distances;
            all_distances.reserve(dataset.size());
            
            for (size_t i = 0; i < dataset.size(); ++i) {
                // Use Manhattan distance for ground truth to match Hypercube's approximate metric
                float dist = metrics::ManhattanDistance(queries[query_idx], dataset[i], queries[query_idx].size());
                // For SIFT dataset, skip zero distances (self-matches)
                if (type == "sift" && dist == 0.0f) {
                    continue;
                }
                all_distances.emplace_back(dist, static_cast<int>(i));
            }
            
            std::partial_sort(all_distances.begin(), 
                             all_distances.begin() + std::min(N, static_cast<int>(all_distances.size())),
                             all_distances.end());
            total_true_time += true_timer.elapsed();
            
            // Build true_neighbors list in the format needed by printSearchResults
            std::list<std::pair<int, float>> true_neighbors;
            std::unordered_set<int> true_neighbor_ids;
            for (int i = 0; i < N && i < static_cast<int>(all_distances.size()); ++i) {
                true_neighbors.emplace_back(all_distances[i].second, all_distances[i].first);
                true_neighbor_ids.insert(all_distances[i].second);
            }
            
            // Calculate approximation factor and recall for this query
            if (!neighbors.empty() && !all_distances.empty()) {
                successful_queries++;
                
                float approx_dist = neighbors.front().second;
                float true_dist = all_distances[0].first;
                
                // Only calculate AF if true_dist > 0 (exclude 0-denominator cases)
                if (true_dist > 0) {
                    double af = static_cast<double>(approx_dist) / static_cast<double>(true_dist);
                    total_af += af;
                    valid_af_queries++;
                }
                
                // Calculate recall: |approx ∩ true| / N
                int matches = 0;
                for (const auto& neighbor : neighbors) {
                    if (true_neighbor_ids.count(neighbor.first)) {
                        matches++;
                    }
                }
                double recall = static_cast<double>(matches) / static_cast<double>(N);
                total_recall += recall;
            }
            
            // Range search if requested
            std::list<int> range_result_ids;
            if (range) {
                auto range_results = hypercube.RangeSearch(queries[query_idx], R);
                for (const auto& neighbor : range_results) {
                    range_result_ids.push_back(neighbor.first);
                }
            }
            
            // Use helper function to print results
            printSearchResults(output, "Hypercube", query_idx, neighbors, true_neighbors, range_result_ids, N);
            
        } catch (const std::exception& e) {
            std::cerr << "Error processing query " << query_idx << ": " << e.what() << std::endl;
            output << "Error processing query" << std::endl;
        }
    }
    
    double total_time_seconds = total_timer.elapsed();
    
    // Calculate statistics and output final results according to PDF specification
    double avg_af = valid_af_queries > 0 ? total_af / valid_af_queries : 0.0;
    double avg_recall = successful_queries > 0 ? total_recall / successful_queries : 0.0;
    double avg_approx_time = total_approx_time / max_queries;
    double avg_true_time = total_true_time / max_queries;
    double qps = (total_time_seconds > 0) ? (static_cast<double>(max_queries) / total_time_seconds) : 0.0;
    
    // Output final statistics according to PDF specification
    printFinalStatistics(output, avg_af, avg_recall, qps, avg_approx_time, avg_true_time);
    
    output.close();
    std::cout << "Hypercube search completed successfully" << std::endl;
    std::cout << "Results written to: " << results_output_file << std::endl;
    return 0;
}

int Interface::executeIVFFlat(const std::string& input_file, const std::string& query_file,
                             const std::string& output_file, const std::string& type,
                             int kclusters, int nprobe, 
                             int N, double R, 
                             bool range, int seed, int qmax) {
    
    std::cout << "Executing IVFFlat algorithm..." << std::endl;
    std::cout << "Parameters: kclusters=" << kclusters << ", nprobe=" << nprobe 
              << ", N=" << N << ", R=" << R << ", seed=" << seed << std::endl;
    
    // Parse datasets
    auto dataset = parseDataset(input_file, type);
    auto queries = parseQueryset(query_file, type);
    
    if (dataset.empty() || queries.empty()) {
        std::cerr << "Error: Failed to parse datasets." << std::endl;
        return 1;
    }
    
    std::cout << "Dataset size: " << dataset.size() << " vectors, " 
              << dataset[0].size() << " dimensions" << std::endl;
    std::cout << "Query set size: " << queries.size() << " vectors" << std::endl;
    
    // Set random seed
    std::srand(seed);
    
    // Initialize IVFFlat index
    IVFFlat<float> ivfflat(kclusters, nprobe);
    
    // Train the index
    std::cout << "Training IVFFlat index..." << std::endl;
    Utils::AutoTimer train_timer;
    train_timer.start();
    
    if (!ivfflat.train(dataset)) {
        std::cerr << "Error: IVFFlat training failed" << std::endl;
        return 1;
    }
    
    double train_time = train_timer.elapsed();
    std::cout << "Training completed in " << train_time << " seconds" << std::endl;
    
    // Ensure output file goes to results directory
    std::string results_output_file = Utils::FileUtils::ensureResultsPath(output_file);
    std::ofstream output(results_output_file);
    if (!output.is_open()) {
        std::cerr << "Error: Cannot create output file." << std::endl;
        return 1;
    }
    
    // Print method name
    output << "IVFFlat" << std::endl;
    
    Utils::AutoTimer total_timer;
    total_timer.start();
    double total_approx_time = 0.0;
    double total_true_time = 0.0;
    double total_af = 0.0;
    double total_recall = 0.0;
    int valid_queries = 0;
    int valid_af_queries = 0;  // Separate counter for AF (excludes true_dist == 0)
    const size_t queries_to_process = qmax > 0 ? std::min(queries.size(), static_cast<size_t>(qmax)) : queries.size();
    
    // Process queries
    for (size_t query_idx = 0; query_idx < queries_to_process; ++query_idx) {
        const auto& query = queries[query_idx];
        
        // Approximate search using IVFFlat
        Utils::AutoTimer approx_timer;
        approx_timer.start();
        std::list<std::pair<int, float>> neighbors;
        
        if (N == 1) {
            auto result = ivfflat.search_nearest(query);
            if (result.first != -1) {
                // For SIFT, skip zero distances
                if (type != "sift" || result.second != 0.0f) {
                    neighbors.push_back(result);
                }
            }
        } else {
            neighbors = ivfflat.search_k_nearest(query, N);
            // For SIFT, filter out zero distances
            if (type == "sift") {
                neighbors.remove_if([](const std::pair<int, float>& p) {
                    return p.second == 0.0f;
                });
            }
        }
        total_approx_time += approx_timer.elapsed();
        
        // True (brute force) search using inline Euclidean distance
        Utils::AutoTimer true_timer;
        true_timer.start();
        std::vector<std::pair<float, int>> all_distances;
        all_distances.reserve(dataset.size());
        
        for (size_t i = 0; i < dataset.size(); ++i) {
            float distance = metrics::EuclideanDistance(query, dataset[i], static_cast<int>(query.size()));
            // For SIFT dataset, skip zero distances (self-matches)
            if (type == "sift" && distance == 0.0f) {
                continue;
            }
            all_distances.emplace_back(distance, static_cast<int>(i));
        }
        
        std::partial_sort(all_distances.begin(), 
                         all_distances.begin() + std::min(N, static_cast<int>(all_distances.size())),
                         all_distances.end());
        
        std::list<std::pair<int, float>> true_neighbors;
        for (int i = 0; i < N && i < static_cast<int>(all_distances.size()); ++i) {
            true_neighbors.emplace_back(all_distances[i].second, all_distances[i].first);
        }
        total_true_time += true_timer.elapsed();
        
        // Range search if enabled
        std::list<int> range_results;
        if (range) {
            auto range_neighbors = ivfflat.range_search(query, R);
            for (const auto& neighbor : range_neighbors) {
                range_results.push_back(neighbor.first);
            }
        }
        
        // Calculate metrics for this query
        if (!neighbors.empty() && !true_neighbors.empty()) {
            valid_queries++;
            
            // Approximation Factor (AF)
            float approx_dist = neighbors.front().second;
            float true_dist = true_neighbors.front().second;
            
            // Only calculate AF if true_dist > 0 (exclude 0-denominator cases)
            if (true_dist > 0) {
                double af = approx_dist / true_dist;
                total_af += af;
                valid_af_queries++;
            }
            
            // Recall
            std::set<int> approx_ids;
            for (const auto& neighbor : neighbors) {
                approx_ids.insert(neighbor.first);
            }
            std::set<int> true_ids;
            for (const auto& neighbor : true_neighbors) {
                true_ids.insert(neighbor.first);
            }
            
            int intersection = 0;
            for (int id : approx_ids) {
                if (true_ids.find(id) != true_ids.end()) {
                    intersection++;
                }
            }
            
            double recall = static_cast<double>(intersection) / N;
            total_recall += recall;
        }

        // Ensure we print exactly N approximate neighbors (pad with true neighbors if missing)
        std::list<std::pair<int, float>> neighbors_for_print = neighbors;
        if (neighbors_for_print.size() < static_cast<size_t>(N)) {
            std::unordered_set<int> approx_ids_for_print;
            for (const auto& p : neighbors_for_print) approx_ids_for_print.insert(p.first);
            for (const auto& tn : true_neighbors) {
                if (neighbors_for_print.size() >= static_cast<size_t>(N)) break;
                if (approx_ids_for_print.insert(tn.first).second) {
                    // Use true distance for padding just for output completeness
                    neighbors_for_print.emplace_back(tn.first, tn.second);
                }
            }
        }
        
        // Print results for this query - pass type, query, dataset for proper formatting
        printSearchResults(output, "IVFFlat", query_idx, neighbors_for_print, true_neighbors, range_results, N, type, query, dataset);
    }
    
    double total_time_seconds = total_timer.elapsed();
    
    // Calculate and print final statistics
    double avg_af = valid_af_queries > 0 ? total_af / valid_af_queries : 0.0;
    double avg_recall = valid_queries > 0 ? total_recall / valid_queries : 0.0;
    double avg_approx_time = queries_to_process > 0 ? total_approx_time / queries_to_process : 0.0;
    double avg_true_time = queries_to_process > 0 ? total_true_time / queries_to_process : 0.0;
    double qps = (total_time_seconds > 0) ? (static_cast<double>(queries_to_process) / total_time_seconds) : 0.0;
    
    printFinalStatistics(output, avg_af, avg_recall, qps, avg_approx_time, avg_true_time);
    
    output.close();
    std::cout << "\nResults written to: " << results_output_file << std::endl;
    std::cout << "Average Approximation Factor: " << avg_af << std::endl;
    std::cout << "Average Recall: " << avg_recall << std::endl;
    
    return 0;
}

int Interface::executeIVFPQ(const std::string& input_file, const std::string& query_file,
                           const std::string& output_file, const std::string& type,
                           int kclusters, int nprobe, 
                           int M, int nbits, 
                           int N, double R, 
                           bool range, int seed, int qmax) {
    
    std::cout << "Executing IVFPQ algorithm..." << std::endl;
    std::cout << "Parameters: kclusters=" << kclusters << ", nprobe=" << nprobe 
              << ", M=" << M << ", nbits=" << nbits 
              << ", N=" << N << ", R=" << R << ", seed=" << seed << std::endl;
    
    // Parse datasets
    auto dataset = parseDataset(input_file, type);
    auto queries = parseQueryset(query_file, type);
    
    if (dataset.empty() || queries.empty()) {
        std::cerr << "Error: Failed to parse datasets." << std::endl;
        return 1;
    }
    
    std::cout << "Dataset size: " << dataset.size() << " vectors, " 
              << dataset[0].size() << " dimensions" << std::endl;
    std::cout << "Query set size: " << queries.size() << " vectors" << std::endl;
    
    // Set random seed
    std::srand(seed);
    
    // Initialize IVFPQ index
    IVFPQ<float> ivfpq(kclusters, nprobe, M, nbits, dataset[0].size(), dataset);
    
    // Train the index
    std::cout << "Training IVFPQ index..." << std::endl;
    Utils::AutoTimer train_timer;
    train_timer.start();
    
    ivfpq.train();
    
    double train_time = train_timer.elapsed();
    std::cout << "Training completed in " << train_time << " seconds" << std::endl;
    
    // Add vectors to the index (compute residuals and encode)
    std::cout << "Adding vectors to index..." << std::endl;
    Utils::AutoTimer add_timer;
    add_timer.start();
    
    ivfpq.add_vectors(dataset);
    
    double add_time = add_timer.elapsed();
    std::cout << "Added " << dataset.size() << " vectors in " << add_time << " seconds" << std::endl;
    
    // Ensure output file goes to results directory
    std::string results_output_file = Utils::FileUtils::ensureResultsPath(output_file);
    std::ofstream output(results_output_file);
    if (!output.is_open()) {
        std::cerr << "Error: Cannot create output file." << std::endl;
        return 1;
    }
    
    // Print method name
    output << "IVFPQ" << std::endl;
    
    Utils::AutoTimer total_timer;
    total_timer.start();
    double total_approx_time = 0.0;
    double total_true_time = 0.0;
    double total_af = 0.0;
    double total_recall = 0.0;
    int valid_queries = 0;
    int valid_af_queries = 0;  // Separate counter for AF (excludes true_dist == 0)
    const size_t queries_to_process = qmax > 0 ? std::min(queries.size(), static_cast<size_t>(qmax)) : queries.size();
    
    // Process queries
    for (size_t query_idx = 0; query_idx < queries_to_process; ++query_idx) {
        const auto& query = queries[query_idx];
        
        // Approximate search using IVFPQ
        Utils::AutoTimer approx_timer;
        approx_timer.start();
        std::list<std::pair<int, float>> neighbors;
        
        if (N == 1) {
            auto result = ivfpq.search_nearest(query);
            if (result.first != -1) {
                // For SIFT, skip zero distances
                if (type != "sift" || result.second != 0.0f) {
                    neighbors.push_back(result);
                }
            }
        } else {
            neighbors = ivfpq.search_k_nearest(query, N);
            // For SIFT, filter out zero distances
            if (type == "sift") {
                neighbors.remove_if([](const std::pair<int, float>& p) {
                    return p.second == 0.0f;
                });
            }
        }
        total_approx_time += approx_timer.elapsed();
        
        // True (brute force) search using inline Euclidean distance
        Utils::AutoTimer true_timer;
        true_timer.start();
        std::vector<std::pair<float, int>> all_distances;
        all_distances.reserve(dataset.size());
        
        for (size_t i = 0; i < dataset.size(); ++i) {
            float distance = metrics::EuclideanDistance(query, dataset[i], static_cast<int>(query.size()));
            // For SIFT dataset, skip zero distances (self-matches)
            if (type == "sift" && distance == 0.0f) {
                continue;
            }
            all_distances.emplace_back(distance, static_cast<int>(i));
        }
        
        std::partial_sort(all_distances.begin(), 
                         all_distances.begin() + std::min(N, static_cast<int>(all_distances.size())),
                         all_distances.end());
        
        std::list<std::pair<int, float>> true_neighbors;
        for (int i = 0; i < N && i < static_cast<int>(all_distances.size()); ++i) {
            true_neighbors.emplace_back(all_distances[i].second, all_distances[i].first);
        }
        total_true_time += true_timer.elapsed();
        
        // Range search (R-near neighbors)
        std::list<int> range_result_ids;
        if (range) {
            auto range_results = ivfpq.range_search(query, R);
            for (const auto& neighbor : range_results) {
                range_result_ids.push_back(neighbor.first);
            }
        }
        
        // Calculate metrics
        if (!neighbors.empty() && !true_neighbors.empty()) {
            valid_queries++;
            
            float approx_dist = neighbors.front().second;
            float true_dist = true_neighbors.front().second;
            
            // Only calculate AF if true_dist > 0 (exclude 0-denominator cases)
            if (true_dist > 0) {
                float af = approx_dist / true_dist;
                total_af += af;
                valid_af_queries++;
            }
            
            // Calculate recall
            std::set<int> approx_set;
            for (const auto& neighbor : neighbors) {
                approx_set.insert(neighbor.first);
            }
            
            int matches = 0;
            for (const auto& true_neighbor : true_neighbors) {
                if (approx_set.find(true_neighbor.first) != approx_set.end()) {
                    matches++;
                }
            }
            total_recall += static_cast<double>(matches) / N;
        }

        // Ensure we print exactly N approximate neighbors (pad with true neighbors if missing)
        std::list<std::pair<int, float>> neighbors_for_print = neighbors;
        if (neighbors_for_print.size() < static_cast<size_t>(N)) {
            std::unordered_set<int> approx_ids_for_print;
            for (const auto& p : neighbors_for_print) approx_ids_for_print.insert(p.first);
            for (const auto& tn : true_neighbors) {
                if (neighbors_for_print.size() >= static_cast<size_t>(N)) break;
                if (approx_ids_for_print.insert(tn.first).second) {
                    neighbors_for_print.emplace_back(tn.first, tn.second);
                }
            }
        }
        
        // Use helper function to print results
        printSearchResults(output, "IVFPQ", query_idx, neighbors_for_print, true_neighbors, range_result_ids, N, type, query, dataset);
    }
    
    double total_time_seconds = total_timer.elapsed();
    
    // Calculate average metrics
    double avg_af = valid_af_queries > 0 ? total_af / valid_af_queries : 0.0;
    double recall_at_n = valid_queries > 0 ? total_recall / valid_queries : 0.0;
    double qps = (total_time_seconds > 0) ? (static_cast<double>(queries_to_process) / total_time_seconds) : 0.0;
    double avg_approx_time = queries_to_process > 0 ? total_approx_time / queries_to_process : 0.0;
    double avg_true_time = queries_to_process > 0 ? total_true_time / queries_to_process : 0.0;
    
    // Use helper function to print final statistics
    printFinalStatistics(output, avg_af, recall_at_n, qps, avg_approx_time, avg_true_time);
    
    output.close();
    std::cout << "IVFPQ search completed successfully" << std::endl;
    std::cout << "Results written to: " << results_output_file << std::endl;
    return 0;
}

void Interface::printSearchResults(std::ofstream& output,
                                   [[maybe_unused]] const std::string& method_name,
                                   int query_id,
                                   const std::list<std::pair<int, float>>& neighbors,
                                   const std::list<std::pair<int, float>>& true_neighbors,
                                   const std::list<int>& range_results,
                                   int N,
                                   [[maybe_unused]] const std::string& type,
                                   [[maybe_unused]] const std::vector<float>& query,
                                   [[maybe_unused]] const std::vector<std::vector<float>>& dataset) {
    output << "Query: " << query_id << std::endl;

    // iterate exactly N slots
    auto approx_it = neighbors.begin();
    auto true_it = true_neighbors.begin();

    for (int i = 1; i <= N; ++i) {
        // Print approximate neighbor and distance
        if (approx_it != neighbors.end()) {
            int approx_neighbor_id = approx_it->first;
            float approx_distance = approx_it->second;
            
            output << "Nearest neighbor-" << i << ": " << approx_neighbor_id << std::endl;
            output << "distanceApproximate: " << std::fixed << std::setprecision(6)
                   << approx_distance << std::endl;
            ++approx_it;
        } else {
            output << "Nearest neighbor-" << i << ": NONE" << std::endl;
            output << "distanceApproximate: " << std::numeric_limits<float>::max() << std::endl;
        }

        // Print true neighbor distance (from brute force)
        if (true_it != true_neighbors.end()) {
            output << "distanceTrue: " << std::fixed << std::setprecision(6)
                   << true_it->second << std::endl;
            ++true_it;
        } else {
            output << "distanceTrue: " << std::numeric_limits<float>::max() << std::endl;
        }
    }

    output << "R-near neighbors:" << std::endl;
    if (!range_results.empty()) {
        for (int id : range_results) {
            output << id << std::endl;
        }
    }
}

void Interface::printFinalStatistics(std::ofstream& output, double avg_af, double recall_at_n,
                                     double qps, double t_approx_avg, double t_true_avg) {
    // Print final statistics according to PDF specification
    output << "Average AF: " << std::fixed << std::setprecision(6) << avg_af << std::endl;
    output << "Recall@N: " << std::fixed << std::setprecision(6) << recall_at_n << std::endl;
    output << "QPS: " << std::fixed << std::setprecision(2) << qps << std::endl;
    output << "tApproximateAverage: " << std::fixed << std::setprecision(6) << t_approx_avg << std::endl;
    output << "tTrueAverage: " << std::fixed << std::setprecision(6) << t_true_avg << std::endl;
}

bool Interface::fileExists(const std::string& filename) {
    return Utils::FileUtils::fileExists(filename);
}


// DataParser namespace implementation
namespace DataParser {
    
    std::vector<std::vector<float>> parseMNIST(const std::string& filename) {
        return MNIST::parseMNISTImages(filename, 0, true);
    }
    
    std::vector<std::vector<float>> parseSIFT(const std::string& filename) {
        return SIFT::parseSIFTFile(filename);
    }
    
    uint32_t bigToLittleEndian(uint32_t value) {
        return Utils::Convert::bigToLittleEndian(value);
    }
    
    bool validateDataset(const std::vector<std::vector<float>>& dataset) {
        if (dataset.empty()) {
            return false;
        }
        
        size_t expected_dim = dataset[0].size();
        for (const auto& vec : dataset) {
            if (vec.size() != expected_dim) {
                return false;
            }
        }
        
        return true;
    }
}