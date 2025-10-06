#include "SIFT.h"
#include "../../utils/math/math.h"
#include "../../metrics/metrics.h"
#include <iostream>
#include <algorithm>
#include <numeric>

namespace SIFT {
    
    std::vector<std::vector<float>> parseSIFTFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open SIFT file: " + filename);
        }
        
        std::vector<std::vector<float>> dataset;
        
        // Read SIFT format: each vector starts with dimension, then the values
        while (file.good()) {
            uint32_t dimension;
            if (!file.read(reinterpret_cast<char*>(&dimension), sizeof(dimension))) {
                break;
            }
            
            // SIFT vectors are typically 128-dimensional
            if (dimension > 1000) { // Sanity check
                std::cerr << "Warning: Unusual dimension size: " << dimension << std::endl;
            }
            
            std::vector<float> vector;
            vector.reserve(dimension);
            
            for (uint32_t i = 0; i < dimension; ++i) {
                float value;
                if (!file.read(reinterpret_cast<char*>(&value), sizeof(value))) {
                    throw std::runtime_error("Error reading SIFT vector data at dimension " + std::to_string(i));
                }
                vector.push_back(value);
            }
            
            if (!vector.empty()) {
                dataset.push_back(std::move(vector));
            }
        }
        
        if (dataset.empty()) {
            throw std::runtime_error("No SIFT vectors found in file: " + filename);
        }
        
        std::cout << "Loaded " << dataset.size() << " SIFT vectors of dimension " 
                  << dataset[0].size() << " from " << filename << std::endl;
        
        return dataset;
    }
    
    std::vector<std::vector<double>> parseSIFTFileDouble(const std::string& filename) {
        auto float_vectors = parseSIFTFile(filename);
        return convertPrecision<float, double>(float_vectors);
    }
    
    void writeSIFTFile(const std::vector<std::vector<float>>& vectors, const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot create SIFT file: " + filename);
        }
        
        for (const auto& vector : vectors) {
            uint32_t dimension = static_cast<uint32_t>(vector.size());
            file.write(reinterpret_cast<const char*>(&dimension), sizeof(dimension));
            
            for (float value : vector) {
                file.write(reinterpret_cast<const char*>(&value), sizeof(value));
            }
        }
        
        std::cout << "Written " << vectors.size() << " SIFT vectors to " << filename << std::endl;
    }
    
    bool validateSIFTDataset(const std::vector<std::vector<float>>& dataset) {
        if (dataset.empty()) {
            std::cerr << "Error: Empty SIFT dataset" << std::endl;
            return false;
        }
        
        size_t expected_dim = dataset[0].size();
        
        // Check if all vectors have the same dimension
        for (size_t i = 0; i < dataset.size(); ++i) {
            if (dataset[i].size() != expected_dim) {
                std::cerr << "Error: Vector " << i << " has dimension " << dataset[i].size() 
                          << ", expected " << expected_dim << std::endl;
                return false;
            }
            
            // Check for NaN or infinite values
            for (size_t j = 0; j < dataset[i].size(); ++j) {
                if (!std::isfinite(dataset[i][j])) {
                    std::cerr << "Error: Non-finite value in vector " << i 
                              << " at dimension " << j << std::endl;
                    return false;
                }
            }
        }
        
        return true;
    }
    
    void printSIFTStatistics(const std::vector<std::vector<float>>& dataset) {
        if (dataset.empty()) {
            std::cout << "Empty SIFT dataset" << std::endl;
            return;
        }
        
        std::cout << "=== SIFT Dataset Statistics ===" << std::endl;
        std::cout << "Number of vectors: " << dataset.size() << std::endl;
        std::cout << "Vector dimension: " << dataset[0].size() << std::endl;
        
        // Calculate min, max, mean values
        float min_val = dataset[0][0];
        float max_val = dataset[0][0];
        double sum = 0.0;
        size_t total_elements = 0;
        
        for (const auto& vector : dataset) {
            for (float val : vector) {
                min_val = std::min(min_val, val);
                max_val = std::max(max_val, val);
                sum += val;
                total_elements++;
            }
        }
        
        double mean = sum / total_elements;
        
        // Calculate standard deviation
        double variance_sum = 0.0;
        for (const auto& vector : dataset) {
            for (float val : vector) {
                double diff = val - mean;
                variance_sum += diff * diff;
            }
        }
        double stddev = std::sqrt(variance_sum / total_elements);
        
        std::cout << "Value range: [" << min_val << ", " << max_val << "]" << std::endl;
        std::cout << "Mean value: " << mean << std::endl;
        std::cout << "Standard deviation: " << stddev << std::endl;
        
        // Calculate average vector magnitude using basic math
        double avg_magnitude = 0.0;
        for (const auto& vector : dataset) {
            double magnitude = 0.0;
            for (float val : vector) {
                magnitude += val * val;
            }
            magnitude = std::sqrt(magnitude);
            avg_magnitude += magnitude;
        }
        avg_magnitude /= dataset.size();
        std::cout << "Average vector magnitude: " << avg_magnitude << std::endl;
        
        std::cout << "==============================" << std::endl;
    }
    
    std::vector<std::vector<float>> normalizeSIFT(const std::vector<std::vector<float>>& vectors) {
        std::vector<std::vector<float>> normalized;
        normalized.reserve(vectors.size());
        
        for (const auto& vector : vectors) {
            // Normalize vector to unit length
            double magnitude = 0.0;
            for (float val : vector) {
                magnitude += val * val;
            }
            magnitude = std::sqrt(magnitude);
            
            std::vector<float> norm_vector;
            norm_vector.reserve(vector.size());
            
            if (magnitude > 0) {
                for (float val : vector) {
                    norm_vector.push_back(val / static_cast<float>(magnitude));
                }
            } else {
                norm_vector = vector; // Keep original if magnitude is 0
            }
            
            normalized.push_back(std::move(norm_vector));
        }
        
        return normalized;
    }
}