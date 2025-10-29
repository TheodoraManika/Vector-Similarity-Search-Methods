#include "SIFT.h"
#include <iostream>
#include <cstdint>

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
        
        return dataset;
    }
}