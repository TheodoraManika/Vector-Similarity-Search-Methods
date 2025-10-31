#include "Utils.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <iomanip>
#include <sstream>
#include <iostream>
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
    
    // AutoTimer implementation
    AutoTimer::AutoTimer(const std::string& timer_name, bool print)
        : name(timer_name), print_on_destroy(print) {
        start();
    }
    
    AutoTimer::~AutoTimer() {
        if (print_on_destroy && !name.empty()) {
            std::cout << name << " took " << elapsed() << " seconds" << std::endl;
        }
    }
    
    void AutoTimer::start() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    double AutoTimer::elapsed() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000000.0;
    }
    
    double AutoTimer::elapsedMs() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
}
