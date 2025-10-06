#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <iostream>

/**
 * SIFT (Scale-Invariant Feature Transform) Data Parser
 * 
 * Handles parsing of SIFT feature vectors from binary files.
 * SIFT vectors are typically 128-dimensional float vectors representing
 * local image features that are invariant to scale, rotation, and illumination.
 */
namespace SIFT {
    
    /**
     * Parse SIFT vectors from binary file
     * @param filename Path to SIFT binary file
     * @return Vector of SIFT feature vectors
     */
    std::vector<std::vector<float>> parseSIFTFile(const std::string& filename);
    
    /**
     * Parse SIFT vectors from binary file (double precision)
     * @param filename Path to SIFT binary file
     * @return Vector of SIFT feature vectors in double precision
     */
    std::vector<std::vector<double>> parseSIFTFileDouble(const std::string& filename);
    
    /**
     * Write SIFT vectors to binary file
     * @param vectors SIFT feature vectors to write
     * @param filename Output file path
     */
    void writeSIFTFile(const std::vector<std::vector<float>>& vectors, const std::string& filename);
    
    /**
     * Validate SIFT dataset dimensions
     * @param dataset SIFT feature vectors
     * @return True if all vectors have same dimension
     */
    bool validateSIFTDataset(const std::vector<std::vector<float>>& dataset);
    
    /**
     * Get statistics of SIFT dataset
     * @param dataset SIFT feature vectors
     */
    void printSIFTStatistics(const std::vector<std::vector<float>>& dataset);
    
    /**
     * Normalize SIFT vectors (L2 normalization)
     * @param vectors SIFT vectors to normalize
     * @return Normalized SIFT vectors
     */
    std::vector<std::vector<float>> normalizeSIFT(const std::vector<std::vector<float>>& vectors);
    
    /**
     * Convert SIFT vectors to different precision
     * @param vectors Input vectors
     * @return Converted vectors
     */
    template<typename InputType, typename OutputType>
    std::vector<std::vector<OutputType>> convertPrecision(const std::vector<std::vector<InputType>>& vectors) {
        std::vector<std::vector<OutputType>> result;
        result.reserve(vectors.size());
        
        for (const auto& vec : vectors) {
            std::vector<OutputType> converted;
            converted.reserve(vec.size());
            
            for (const auto& val : vec) {
                converted.push_back(static_cast<OutputType>(val));
            }
            
            result.push_back(std::move(converted));
        }
        
        return result;
    }
}