#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <memory>

// Forward declarations
template<typename T> class LSH;
template<typename T> class Hypercube;
template<typename T> class IVFFlat;
template<typename T> class IVFPQ;

/**
 * Main interface class for vector similarity search operations
 */
class Interface {
public:
    Interface();
    ~Interface();
    
    // Main program entry point
    int runSearch(int argc, char* argv[]);
    
    // Input parsing functions
    std::vector<std::vector<float>> parseDataset(const std::string& filename, const std::string& type);
    std::vector<std::vector<float>> parseQueryset(const std::string& filename, const std::string& type);
    
    // Command line argument parsing
    void parseSearchArgs(int argc, char* argv[], std::string& input_file, 
                        std::string& query_file, std::string& output_file,
                        std::string& type, std::string& method,
                        int& k, int& L, int& kproj, int& kclusters, int& nprobe,
                        int& M, int& probes, int& nbits, int& N, int& seed,
                        double& w, double& R, bool& range, int& qmax, bool& auto_w);
    
    // Algorithm execution functions
    int executeLSH(const std::string& input_file, const std::string& query_file,
                  const std::string& output_file, const std::string& type,
                  int k, int L, double w, int N, double R, bool range, int qmax, bool auto_w);
    
    int executeHypercube(const std::string& input_file, const std::string& query_file,
                        const std::string& output_file, const std::string& type,
                        int kproj, double w, int M, int probes, int N, double R, bool range, int seed, int qmax, bool auto_w);
    
    int executeIVFFlat(const std::string& input_file, const std::string& query_file,
                      const std::string& output_file, const std::string& type,
                                            int kclusters, int nprobe, int N, double R, bool range, int seed, int qmax);
    
    int executeIVFPQ(const std::string& input_file, const std::string& query_file,
                    const std::string& output_file, const std::string& type,
                                        int kclusters, int nprobe, int M, int nbits, int N, double R, bool range, int seed, int qmax);
    
    // Output functions following exact assignment format
    void printSearchResults(std::ofstream& output,
                                   const std::string& method_name,
                                   int query_id,
                                   const std::list<std::pair<int, float>>& neighbors,
                                   const std::list<std::pair<int, float>>& true_neighbors,
                                   const std::list<int>& range_results,
                                   int N,
                                   const std::string& type = "",
                                   const std::vector<float>& query = std::vector<float>(),
                                   const std::vector<std::vector<float>>& dataset = std::vector<std::vector<float>>());
    
    void printFinalStatistics(std::ofstream& output, double avg_af, double recall_at_n,
                                     double qps, double t_approx_avg, double t_true_avg);
    
    // Utility functions
    bool fileExists(const std::string& filename);
};

// Global utility functions for data parsing
namespace DataParser {
    // Parse MNIST binary files
    std::vector<std::vector<float>> parseMNIST(const std::string& filename);
    
    // Parse SIFT binary files 
    std::vector<std::vector<float>> parseSIFT(const std::string& filename);
    
    // Endianness conversion utilities
    uint32_t bigToLittleEndian(uint32_t value);
    
    // Data validation
    bool validateDataset(const std::vector<std::vector<float>>& dataset);
}