#include "Utils.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace Utils {
    
    // File utilities implementation
    namespace FileUtils {
        
        bool fileExists(const std::string& filepath) {
            struct stat buffer;
            return (stat(filepath.c_str(), &buffer) == 0);
        }
        
        std::string getCurrentTimestamp() {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
            return ss.str();
        }
        
        std::string ensureResultsPath(const std::string& filename) {
            // If filename already starts with "results/", return as is
            if (filename.find("results/") == 0) {
                return filename;
            }
            
            // Extract just the filename (remove any path components)
            size_t last_slash = filename.find_last_of("/\\");
            std::string basename = (last_slash != std::string::npos) 
                ? filename.substr(last_slash + 1) 
                : filename;
            
            // Create the results path
            std::string results_path = "results/" + basename;
            
            // Ensure the results directory exists
            struct stat st;
            if (stat("results", &st) != 0) {
                mkdir("results", 0755);
            }
            
            return results_path;
        }
    }
    
    // Statistics utilities implementation
    namespace Stats {
        
        double calculateAF(float approx_dist, float true_dist) {
            if (true_dist == 0.0f) return 1.0;
            return static_cast<double>(approx_dist) / static_cast<double>(true_dist);
        }
        
        double calculateRecallAtN(
            const std::vector<std::list<std::pair<int, float>>>& approx_results,
            const std::vector<std::list<std::pair<int, float>>>& true_results,
            int N
        ) {
            size_t num_queries = approx_results.size();
            if (num_queries == 0 || num_queries != true_results.size()) {
                return 0.0;
            }
            
            double total_recall = 0.0;
            size_t n = static_cast<size_t>(N);
            
            for (size_t q = 0; q < num_queries; ++q) {
                size_t matches = 0;
                size_t check_size = std::min(n, approx_results[q].size());
                size_t true_size = std::min(n, true_results[q].size());
                
                // Convert list to vector for easier iteration
                std::vector<std::pair<int, float>> approx_vec(approx_results[q].begin(), approx_results[q].end());
                std::vector<std::pair<int, float>> true_vec(true_results[q].begin(), true_results[q].end());
                
                for (size_t i = 0; i < check_size; ++i) {
                    int approx_id = approx_vec[i].first;
                    for (size_t j = 0; j < true_size; ++j) {
                        if (approx_id == true_vec[j].first) {
                            matches++;
                            break;
                        }
                    }
                }
                
                if (true_size > 0) {
                    total_recall += static_cast<double>(matches) / static_cast<double>(true_size);
                }
            }
            
            return total_recall / static_cast<double>(num_queries);
        }
        
        double calculateQPS(int num_queries, double total_time) {
            if (total_time <= 0.0) return 0.0;
            return static_cast<double>(num_queries) / total_time;
        }
    }
    
    // Conversion utilities implementation
    namespace Convert {
        
        uint32_t bigToLittleEndian(uint32_t value) {
            return ((value & 0xFF000000) >> 24) |
                   ((value & 0x00FF0000) >> 8)  |
                   ((value & 0x0000FF00) << 8)  |
                   ((value & 0x000000FF) << 24);
        }
        
        uint16_t bigToLittleEndian(uint16_t value) {
            return ((value & 0xFF00) >> 8) |
                   ((value & 0x00FF) << 8);
        }
    }
}
