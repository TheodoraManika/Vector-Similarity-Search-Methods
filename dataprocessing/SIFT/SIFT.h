#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cstdint>

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
}