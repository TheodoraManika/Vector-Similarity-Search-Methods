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
     * Data conversion utilities
     */
    namespace Convert {
        // Endianness conversion
        uint32_t bigToLittleEndian(uint32_t value);
        uint16_t bigToLittleEndian(uint16_t value);
    }
    
    /**
     * Timer utility for measuring execution time
     */
    class AutoTimer {
    private:
        std::chrono::high_resolution_clock::time_point start_time;
        std::string name;
        bool print_on_destroy;
        
    public:
        AutoTimer(const std::string& timer_name = "", bool print = false);
        ~AutoTimer();
        
        void start();
        double elapsed() const;  // Returns elapsed time in seconds
        double elapsedMs() const;  // Returns elapsed time in milliseconds
    };
}
