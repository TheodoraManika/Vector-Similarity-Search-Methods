#pragma once

#include <vector>
#include <string>
#include <list>
#include <fstream>
#include <chrono>

/**
 * Utility functions for the Vector Similarity Search project
 * Contains helper functions for data processing, statistics, and common operations
 */
namespace Utils {
    
    /**
     * File and I/O utilities
     */
    namespace FileUtils {
        bool fileExists(const std::string& filepath);
        std::string getCurrentTimestamp();
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
    }
    
    /**
     * Data conversion utilities
     */
    namespace Convert {
        // Endianness conversion
        uint32_t bigToLittleEndian(uint32_t value);
        uint16_t bigToLittleEndian(uint16_t value);
    }
}
