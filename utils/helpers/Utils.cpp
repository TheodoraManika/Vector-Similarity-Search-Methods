#include "Utils.h"
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <unordered_set>
#include <cmath>

namespace Utils {
    
    // File utilities implementation
    namespace FileUtils {
        
        bool fileExists(const std::string& filename) {
            std::ifstream file(filename);
            return file.good();
        }
        
        std::string getCurrentTimestamp() {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            return ss.str();
        }
        
        void ensureDirectoryExists(const std::string& filepath) {
            std::filesystem::path path(filepath);
            std::filesystem::path dir = path.parent_path();
            
            if (!dir.empty() && !std::filesystem::exists(dir)) {
                std::filesystem::create_directories(dir);
            }
        }
        
        std::string ensureResultsPath(const std::string& filename) {
            // If filename already starts with "results/", return as is
            if (filename.find("results/") == 0) {
                return filename;
            }
            
            // If filename contains path separators, extract just the filename
            std::filesystem::path path(filename);
            std::string basename = path.filename().string();
            
            // Create the results path
            std::string results_path = "results/" + basename;
            
            // Ensure the results directory exists
            ensureDirectoryExists(results_path);
            
            return results_path;
        }
    }
    
    // Statistics utilities implementation
    namespace Stats {
        
        double calculateAF(float approx_dist, float true_dist) {
            return true_dist > 0 ? static_cast<double>(approx_dist) / static_cast<double>(true_dist) : 1.0;
        }
        
        double calculateRecallAtN(const std::vector<std::list<std::pair<int, float>>>& approx_results,
                                 const std::vector<std::list<std::pair<int, float>>>& true_results, int N) {
            if (approx_results.size() != true_results.size() || approx_results.empty()) {
                return 0.0;
            }
            
            int successful_queries = 0;
            
            for (size_t i = 0; i < approx_results.size(); ++i) {
                if (true_results[i].empty()) continue;
                
                // Get the true nearest neighbor
                int true_nearest = true_results[i].front().first;
                
                // Check if true nearest is in top N approximate results
                int count = 0;
                for (const auto& result : approx_results[i]) {
                    if (count >= N) break;
                    if (result.first == true_nearest) {
                        successful_queries++;
                        break;
                    }
                    count++;
                }
            }
            
            return static_cast<double>(successful_queries) / static_cast<double>(approx_results.size());
        }
        
        double calculateQPS(int num_queries, double total_time) {
            return total_time > 0 ? static_cast<double>(num_queries) / total_time : 0.0;
        }
        
        double calculateAverageAF(const std::vector<std::list<std::pair<int, float>>>& approx_results,
                                 const std::vector<std::list<std::pair<int, float>>>& true_results) {
            if (approx_results.size() != true_results.size() || approx_results.empty()) {
                return 1.0;
            }
            
            double total_af = 0.0;
            int valid_queries = 0;
            
            for (size_t i = 0; i < approx_results.size(); ++i) {
                if (!approx_results[i].empty() && !true_results[i].empty()) {
                    float approx_dist = approx_results[i].front().second;
                    float true_dist = true_results[i].front().second;
                    
                    if (true_dist > 0) {
                        total_af += calculateAF(approx_dist, true_dist);
                        valid_queries++;
                    }
                }
            }
            
            return valid_queries > 0 ? total_af / valid_queries : 1.0;
        }
    }
    
    // Validation utilities implementation
    namespace Validation {
        
        bool validateParameters(int k, int L, double w, int N, double R) {
            if (k <= 0 || L <= 0 || w <= 0 || N <= 0 || R < 0) {
                return false;
            }
            
            // Additional sanity checks
            if (k > 64 || L > 100) { // Reasonable upper bounds
                return false;
            }
            
            return true;
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
    
    // Timer implementation
    Timer::Timer() {
        start();
    }
    
    void Timer::start() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    double Timer::elapsed() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000000.0;
    }
    
    double Timer::elapsedMs() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
    
    // Progress bar implementation
    ProgressBar::ProgressBar(size_t total, int width) : total(total), current(0), width(width) {
    }
    
    void ProgressBar::update(size_t current) {
        this->current = current;
        
        double progress = static_cast<double>(current) / static_cast<double>(total);
        int filled = static_cast<int>(progress * width);
        
        std::cout << "\r[";
        for (int i = 0; i < filled; ++i) {
            std::cout << "#";
        }
        for (int i = filled; i < width; ++i) {
            std::cout << " ";
        }
        std::cout << "] " << std::fixed << std::setprecision(1) << (progress * 100.0) << "%";
        std::cout.flush();
    }
    
    void ProgressBar::finish() {
        update(total);
        std::cout << std::endl;
    }
}