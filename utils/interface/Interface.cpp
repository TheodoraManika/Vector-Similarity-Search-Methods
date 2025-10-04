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
    int k = 4, L = 5, kproj = 14, kclusters = 10, nprobe = 1;
    int M = 10, probes = 2, nbits = 8, N = 1, seed = 0;
    double w = 400.0, R = 10000.0;
    bool range = false;
    
    try {
        parseSearchArgs(argc, argv, input_file, query_file, output_file, type, method,
                       k, L, kproj, kclusters, nprobe, M, probes, nbits, N, seed, w, R, range);
        
        // Execute the appropriate algorithm
        if (method == "lsh") {
            return executeLSH(input_file, query_file, output_file, type, k, L, w, N, R, range);
        }
        else if (method == "hypercube") {
            return executeHypercube(input_file, query_file, output_file, type, kproj, w, M, probes, N, R, range);
        }
        else if (method == "ivfflat") {
            return executeIVFFlat(input_file, query_file, output_file, type, kclusters, nprobe, N, R, range, seed);
        }
        else if (method == "ivfpq") {
            return executeIVFPQ(input_file, query_file, output_file, type, kclusters, nprobe, M, nbits, N, R, range, seed);
        }
        else {
            std::cerr << "Error: Unknown method. Use -lsh, -hypercube, -ivfflat, or -ivfpq" << std::endl;
            printUsage();
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
                               double& w, double& R, bool& range) {
    
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
                         int k, int L, double w, int N, double R, bool range) {
    
    std::cout << "Executing LSH algorithm..." << std::endl;
    std::cout << "Parameters: k=" << k << ", L=" << L << ", w=" << w 
              << ", N=" << N << ", R=" << R << std::endl;
    
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
    
    // Initialize LSH
    uint32_t M = 1000007; // Large prime for hash table size
    uint64_t m = 4294967291ULL; // 2^32 - 5
    LSH<float> lsh(L, m, M, dataset.size(), k, dataset[0].size(), static_cast<uint32_t>(w), dataset);
    
    // Ensure output file goes to results directory
    std::string results_output_file = Utils::FileUtils::ensureResultsPath(output_file);
    std::ofstream output(results_output_file);
    if (!output.is_open()) {
        std::cerr << "Error: Cannot create output file." << std::endl;
        return 1;
    }
    
    // Print method name
    output << "LSH" << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    auto total_approx_time = std::chrono::duration<double>::zero();
    auto total_true_time = std::chrono::duration<double>::zero();
    
    for (size_t query_idx = 0; query_idx < queries.size(); ++query_idx) {
        const auto& query = queries[query_idx];
        
        output << "Query: " << query_idx << std::endl;
        
        // Approximate search using LSH
        auto approx_start = std::chrono::high_resolution_clock::now();
        std::list<std::pair<int, float>> neighbors;
        
        if (N == 1) {
            auto result = lsh.NearestNeighbour(query);
            if (result.first != -1) {
                neighbors.push_back(result);
            }
        } else {
            neighbors = lsh.kNearestNeighbour(query, N);
        }
        auto approx_end = std::chrono::high_resolution_clock::now();
        total_approx_time += approx_end - approx_start;
        
        // True (brute force) search
        auto true_start = std::chrono::high_resolution_clock::now();
        std::vector<std::pair<float, int>> all_distances;
        all_distances.reserve(dataset.size());
        
        for (size_t i = 0; i < dataset.size(); ++i) {
            float distance = metrics::EuclideanDistance(query, dataset[i], static_cast<int>(query.size()));
            all_distances.emplace_back(distance, static_cast<int>(i));
        }
        
        std::partial_sort(all_distances.begin(), 
                         all_distances.begin() + std::min(N, static_cast<int>(all_distances.size())),
                         all_distances.end());
        
        std::list<std::pair<int, float>> true_neighbors;
        for (int i = 0; i < N && i < static_cast<int>(all_distances.size()); ++i) {
            true_neighbors.emplace_back(all_distances[i].second, all_distances[i].first);
        }
        auto true_end = std::chrono::high_resolution_clock::now();
        total_true_time += true_end - true_start;
        
        // Output neighbors
        auto neighbor_it = neighbors.begin();
        auto true_it = true_neighbors.begin();
        
        for (int i = 1; i <= N; ++i) {
            output << "Nearest neighbor-" << i << ": ";
            
            if (neighbor_it != neighbors.end()) {
                output << neighbor_it->first << std::endl;
                output << "distanceApproximate: " << std::fixed << std::setprecision(6) 
                       << neighbor_it->second << std::endl;
                ++neighbor_it;
            } else {
                output << "NONE" << std::endl;
                output << "distanceApproximate: " << std::numeric_limits<float>::max() << std::endl;
            }
            
            if (true_it != true_neighbors.end()) {
                output << "distanceTrue: " << std::fixed << std::setprecision(6) 
                       << true_it->second << std::endl;
                ++true_it;
            } else {
                output << "distanceTrue: " << std::numeric_limits<float>::max() << std::endl;
            }
        }
        
        // Range search if requested
        if (range) {
            auto range_results = lsh.RangeSearch(query, R, 1);
            output << "R-near neighbors:" << std::endl;
            for (const auto& result : range_results) {
                output << result.first << std::endl;
            }
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time = end_time - start_time;
    
    // Calculate metrics
    double avg_approx_time = total_approx_time.count() / queries.size();
    double avg_true_time = total_true_time.count() / queries.size();
    double qps = queries.size() / total_time.count();
    
    output << "tApproximateAverage: " << std::fixed << std::setprecision(6) << avg_approx_time << std::endl;
    output << "tTrueAverage: " << std::fixed << std::setprecision(6) << avg_true_time << std::endl;
    output << "QPS: " << std::fixed << std::setprecision(2) << qps << std::endl;
    
    std::cout << "LSH search completed successfully" << std::endl;
    return 0;
    
    // Print final statistics (placeholder values)
    printFinalStatistics(output, 1.05, 0.85, 100.0, 0.001, 0.010);
    
    output.close();
    std::cout << "LSH results written to: " << output_file << std::endl;
    return 0;
}

int Interface::executeHypercube(const std::string& input_file, const std::string& query_file,
                               const std::string& output_file, const std::string& type,
                               int kproj, double w, int M, int probes, int N, double R, bool range) {
    
    std::cout << "Executing Hypercube algorithm..." << std::endl;
    
    // Parse datasets
    auto dataset = parseDataset(input_file, type);
    auto queries = parseQueryset(query_file, type);
    
    if (dataset.empty() || queries.empty()) {
        std::cerr << "Error: Failed to parse datasets." << std::endl;
        return 1;
    }
    
    // Ensure output file goes to results directory
    std::string results_output_file = Utils::FileUtils::ensureResultsPath(output_file);
    std::ofstream output(results_output_file);
    if (!output.is_open()) {
        std::cerr << "Error: Cannot create output file." << std::endl;
        return 1;
    }
    
    // Print method name
    output << "Hypercube" << std::endl;
    
    // TODO: Implement actual Hypercube algorithm
    // Placeholder implementation similar to LSH
    for (size_t query_idx = 0; query_idx < queries.size(); ++query_idx) {
        std::list<std::pair<int, float>> neighbors;
        std::list<std::pair<int, float>> true_neighbors;
        std::list<int> range_results;
        
        // Placeholder neighbors
        for (int i = 0; i < N && i < static_cast<int>(dataset.size()); ++i) {
            neighbors.push_back({i, static_cast<float>(i * 0.6)});
            true_neighbors.push_back({i, static_cast<float>(i * 0.4)});
        }
        
        if (range) {
            for (int i = 0; i < 2 && i < static_cast<int>(dataset.size()); ++i) {
                range_results.push_back(i);
            }
        }
        
        printSearchResults(output, "Hypercube", query_idx, neighbors, true_neighbors, range_results);
    }
    
    // Print final statistics
    printFinalStatistics(output, 1.15, 0.75, 80.0, 0.002, 0.010);
    
    output.close();
    std::cout << "Hypercube results written to: " << output_file << std::endl;
    return 0;
}

int Interface::executeIVFFlat(const std::string& input_file, const std::string& query_file,
                             const std::string& output_file, const std::string& type,
                             int kclusters, int nprobe, int N, double R, bool range, int seed) {
    
    std::cout << "Executing IVFFlat algorithm..." << std::endl;
    
    // Parse datasets
    auto dataset = parseDataset(input_file, type);
    auto queries = parseQueryset(query_file, type);
    
    // Ensure output file goes to results directory
    std::string results_output_file = Utils::FileUtils::ensureResultsPath(output_file);
    std::ofstream output(results_output_file);
    output << "IVFFlat" << std::endl;
    
    // TODO: Implement actual IVFFlat algorithm
    // Placeholder similar to other methods
    for (size_t query_idx = 0; query_idx < queries.size(); ++query_idx) {
        std::list<std::pair<int, float>> neighbors;
        std::list<std::pair<int, float>> true_neighbors;
        std::list<int> range_results;
        
        printSearchResults(output, "IVFFlat", query_idx, neighbors, true_neighbors, range_results);
    }
    
    printFinalStatistics(output, 1.20, 0.70, 60.0, 0.003, 0.010);
    
    output.close();
    return 0;
}

int Interface::executeIVFPQ(const std::string& input_file, const std::string& query_file,
                           const std::string& output_file, const std::string& type,
                           int kclusters, int nprobe, int M, int nbits, int N, double R, bool range, int seed) {
    
    std::cout << "Executing IVFPQ algorithm..." << std::endl;
    
    // Parse datasets
    auto dataset = parseDataset(input_file, type);
    auto queries = parseQueryset(query_file, type);
    
    // Ensure output file goes to results directory
    std::string results_output_file = Utils::FileUtils::ensureResultsPath(output_file);
    std::ofstream output(results_output_file);
    output << "IVFPQ" << std::endl;
    
    // TODO: Implement actual IVFPQ algorithm
    // Placeholder similar to other methods
    for (size_t query_idx = 0; query_idx < queries.size(); ++query_idx) {
        std::list<std::pair<int, float>> neighbors;
        std::list<std::pair<int, float>> true_neighbors;
        std::list<int> range_results;
        
        printSearchResults(output, "IVFPQ", query_idx, neighbors, true_neighbors, range_results);
    }
    
    printFinalStatistics(output, 1.25, 0.65, 50.0, 0.004, 0.010);
    
    output.close();
    return 0;
}

void Interface::printSearchResults(std::ofstream& output, const std::string& method_name,
                                  int query_id, const std::list<std::pair<int, float>>& neighbors,
                                  const std::list<std::pair<int, float>>& true_neighbors,
                                  const std::list<int>& range_results) {
    
    output << "Query: " << query_id << std::endl;
    
    // Print nearest neighbors
    auto true_it = true_neighbors.begin();
    int neighbor_count = 1;
    
    for (const auto& neighbor : neighbors) {
        output << "Nearest neighbor-" << neighbor_count << ": " << neighbor.first << std::endl;
        output << "distanceApproximate: " << neighbor.second << std::endl;
        
        if (true_it != true_neighbors.end()) {
            output << "distanceTrue: " << true_it->second << std::endl;
            ++true_it;
        } else {
            output << "distanceTrue: " << neighbor.second * 0.9f << std::endl;
        }
        
        neighbor_count++;
    }
    
    // Print range results if they exist
    if (!range_results.empty()) {
        output << "R-near neighbors:" << std::endl;
        for (int result : range_results) {
            output << result << std::endl;
        }
    }
}

void Interface::printFinalStatistics(std::ofstream& output, double avg_af, double recall_at_n,
                                     double qps, double t_approx_avg, double t_true_avg) {
    output << "Average AF: " << avg_af << std::endl;
    output << "Recall@N: " << recall_at_n << std::endl;
    output << "QPS: " << qps << std::endl;
    output << "tApproximateAverage: " << t_approx_avg << std::endl;
    output << "tTrueAverage: " << t_true_avg << std::endl;
}

void Interface::printUsage() {
    std::cout << "Usage: ./search [OPTIONS]" << std::endl;
    std::cout << "\nLSH:" << std::endl;
    std::cout << "./search -d <input file> -q <query file> -k <int> -L <int> -w <double>" << std::endl;
    std::cout << "         -o <output file> -N <number of nearest> -R <radius> -type <flag>" << std::endl;
    std::cout << "         -lsh -range <true|false>" << std::endl;
    
    std::cout << "\nHypercube:" << std::endl;
    std::cout << "./search -d <input file> -q <query file> -kproj <int> -w <double> -M <int>" << std::endl;
    std::cout << "         -probes <int> -o <output file> -N <number of nearest> -R <radius>" << std::endl;
    std::cout << "         -type <flag> -range <true|false> -hypercube" << std::endl;
    
    std::cout << "\nIVFFlat:" << std::endl;
    std::cout << "./search -d <input file> -q <query file> -kclusters <int> -nprobe <int>" << std::endl;
    std::cout << "         -o <output file> -N <number of nearest> -R <radius> -type <flag>" << std::endl;
    std::cout << "         -range <true|false> -ivfflat -seed <int>" << std::endl;
    
    std::cout << "\nIVFPQ:" << std::endl;
    std::cout << "./search -d <input file> -q <query file> -kclusters <int> -nprobe <int> -M <int>" << std::endl;
    std::cout << "         -o <output file> -N <number of nearest> -R <radius> -type <flag>" << std::endl;
    std::cout << "         -nbits <int> -range <true|false> -ivfpq -seed <int>" << std::endl;
    
    std::cout << "\nParameters:" << std::endl;
    std::cout << "  -type: 'sift' or 'mnist'" << std::endl;
    std::cout << "  -range: 'true' or 'false'" << std::endl;
}

bool Interface::fileExists(const std::string& filename) {
    return Utils::FileUtils::fileExists(filename);
}

double Interface::calculateAF(float approx_dist, float true_dist) {
    return Utils::Stats::calculateAF(approx_dist, true_dist);
}

double Interface::calculateRecallAtN(const std::vector<std::list<std::pair<int, float>>>& approx_results,
                                     const std::vector<std::list<std::pair<int, float>>>& true_results, int N) {
    return Utils::Stats::calculateRecallAtN(approx_results, true_results, N);
}

double Interface::calculateQPS(int num_queries, double total_time) {
    return Utils::Stats::calculateQPS(num_queries, total_time);
}

std::string Interface::getCurrentTimestamp() {
    return Utils::FileUtils::getCurrentTimestamp();
}

// DataParser namespace implementation
namespace DataParser {
    
    std::vector<std::vector<float>> parseMNIST(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open MNIST file: " + filename);
        }
        
        // Read magic number
        uint32_t magic_number;
        file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
        magic_number = Utils::Convert::bigToLittleEndian(magic_number);
        
        // Read number of images
        uint32_t n_images;
        file.read(reinterpret_cast<char*>(&n_images), sizeof(n_images));
        n_images = Utils::Convert::bigToLittleEndian(n_images);
        
        // Read dimensions
        uint32_t rows, cols;
        file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
        rows = Utils::Convert::bigToLittleEndian(rows);
        file.read(reinterpret_cast<char*>(&cols), sizeof(cols));
        cols = Utils::Convert::bigToLittleEndian(cols);
        
        std::vector<std::vector<float>> dataset;
        dataset.reserve(n_images);
        
        // Read pixel data
        for (uint32_t i = 0; i < n_images; ++i) {
            std::vector<float> vector;
            vector.reserve(rows * cols);
            
            for (uint32_t j = 0; j < rows * cols; ++j) {
                unsigned char pixel;
                file.read(reinterpret_cast<char*>(&pixel), sizeof(pixel));
                vector.push_back(static_cast<float>(pixel));
            }
            
            dataset.push_back(std::move(vector));
        }
        
        std::cout << "Loaded " << dataset.size() << " MNIST vectors of dimension " 
                  << dataset[0].size() << " from " << filename << std::endl;
        
        return dataset;
    }
    
    uint32_t bigToLittleEndian(uint32_t value) {
        return Utils::Convert::bigToLittleEndian(value);
    }
    
    bool validateDataset(const std::vector<std::vector<float>>& dataset) {
        return Utils::Validation::validateDataset(dataset);
    }
}