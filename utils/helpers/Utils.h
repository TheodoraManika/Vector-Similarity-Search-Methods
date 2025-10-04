#pragma once

#include <vector>
#include <string>
#include <list>
#include <fstream>
#include <chrono>
#include <cmath>

/**
 * Utility functions for the Vector Similarity Search project
 * Contains helper functions for data processing, statistics, and common operations
 */
namespace Utils {
    
    /**
     * File and I/O utilities
     */
    namespace FileUtils {
        bool fileExists(const std::string& filename);
        std::string getCurrentTimestamp();
        void ensureDirectoryExists(const std::string& filepath);
        std::string ensureResultsPath(const std::string& filename);
    }
    
    /**
     * Statistical calculation utilities
     */
    namespace Stats {
        // Calculate approximation factor
        double calculateAF(float approx_dist, float true_dist);
        
        // Calculate Recall@N
        double calculateRecallAtN(const std::vector<std::list<std::pair<int, float>>>& approx_results,
                                 const std::vector<std::list<std::pair<int, float>>>& true_results, int N);
        
        // Calculate queries per second
        double calculateQPS(int num_queries, double total_time);
        
        // Calculate average approximation factor across multiple queries
        double calculateAverageAF(const std::vector<std::list<std::pair<int, float>>>& approx_results,
                                 const std::vector<std::list<std::pair<int, float>>>& true_results);
    }
    
    /**
     * Data validation utilities
     */
    namespace Validation {
        template<typename T>
        bool validateDataset(const std::vector<std::vector<T>>& dataset);
        
        template<typename T>
        bool validateDimensions(const std::vector<std::vector<T>>& dataset, size_t expected_dim);
        
        bool validateParameters(int k, int L, double w, int N, double R);
    }
    
    /**
     * Data conversion utilities
     */
    namespace Convert {
        // Convert between different numeric types
        template<typename From, typename To>
        std::vector<std::vector<To>> convertDataset(const std::vector<std::vector<From>>& dataset);
        
        // Endianness conversion
        uint32_t bigToLittleEndian(uint32_t value);
        uint16_t bigToLittleEndian(uint16_t value);
    }
    
    /**
     * Timer utility for performance measurement
     */
    class Timer {
    private:
        std::chrono::high_resolution_clock::time_point start_time;
        
    public:
        Timer();
        void start();
        double elapsed(); // Returns elapsed time in seconds
        double elapsedMs(); // Returns elapsed time in milliseconds
    };
    
    /**
     * Progress bar utility
     */
    class ProgressBar {
    private:
        size_t total;
        size_t current;
        int width;
        
    public:
        ProgressBar(size_t total, int width = 50);
        void update(size_t current);
        void finish();
    };
    
    // Template implementations
    template<typename T>
    bool Validation::validateDataset(const std::vector<std::vector<T>>& dataset) {
        if (dataset.empty()) return false;
        
        size_t expected_dim = dataset[0].size();
        for (const auto& vector : dataset) {
            if (vector.size() != expected_dim) {
                return false;
            }
            
            // Check for invalid values
            for (const auto& val : vector) {
                if (!std::isfinite(static_cast<double>(val))) {
                    return false;
                }
            }
        }
        return true;
    }
    
    template<typename T>
    bool Validation::validateDimensions(const std::vector<std::vector<T>>& dataset, size_t expected_dim) {
        for (const auto& vector : dataset) {
            if (vector.size() != expected_dim) {
                return false;
            }
        }
        return true;
    }
    
    template<typename From, typename To>
    std::vector<std::vector<To>> Convert::convertDataset(const std::vector<std::vector<From>>& dataset) {
        std::vector<std::vector<To>> result;
        result.reserve(dataset.size());
        
        for (const auto& vec : dataset) {
            std::vector<To> converted;
            converted.reserve(vec.size());
            
            for (const auto& val : vec) {
                converted.push_back(static_cast<To>(val));
            }
            
            result.push_back(std::move(converted));
        }
        
        return result;
    }
}