#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "../../utils/math/math.h"
#include "../../utils/helpers/Utils.h"

/**
 * MNIST Data Parser
 * 
 * Handles parsing of MNIST dataset from binary files.
 * MNIST contains 28x28 grayscale images (784 dimensions) of handwritten digits.
 */
namespace MNIST {
    
    /**
     * Parse MNIST images from binary file
     * @param filename Path to MNIST images binary file
     * @return Vector of image vectors (each image is flattened to 784 values)
     */
    std::vector<std::vector<float>> parseMNISTImages(const std::string& filename);
    
    /**
     * Parse MNIST images from binary file (double precision)
     * @param filename Path to MNIST images binary file
     * @return Vector of image vectors in double precision
     */
    std::vector<std::vector<double>> parseMNISTImagesDouble(const std::string& filename);
    
    /**
     * Parse MNIST labels from binary file
     * @param filename Path to MNIST labels binary file
     * @return Vector of labels (0-9)
     */
    std::vector<int> parseMNISTLabels(const std::string& filename);
    
    /**
     * Write MNIST images to binary file
     * @param images Image vectors to write
     * @param filename Output file path
     */
    void writeMNISTImages(const std::vector<std::vector<float>>& images, const std::string& filename);
    
    /**
     * Validate MNIST dataset dimensions
     * @param dataset MNIST image vectors
     * @return True if all images have 784 dimensions
     */
    bool validateMNISTDataset(const std::vector<std::vector<float>>& dataset);
    
    /**
     * Get statistics of MNIST dataset
     * @param dataset MNIST image vectors
     */
    void printMNISTStatistics(const std::vector<std::vector<float>>& dataset);
    
    /**
     * Normalize MNIST images (pixel values 0-255 to 0-1)
     * @param images MNIST images to normalize
     * @return Normalized MNIST images
     */
    std::vector<std::vector<float>> normalizeMNIST(const std::vector<std::vector<float>>& images);
    
    /**
     * Convert MNIST images to different precision
     * @param images Input images
     * @return Converted images
     */
    template<typename InputType, typename OutputType>
    std::vector<std::vector<OutputType>> convertPrecision(const std::vector<std::vector<InputType>>& images) {
        std::vector<std::vector<OutputType>> result;
        result.reserve(images.size());
        
        for (const auto& img : images) {
            std::vector<OutputType> converted;
            converted.reserve(img.size());
            
            for (const auto& val : img) {
                converted.push_back(static_cast<OutputType>(val));
            }
            
            result.push_back(std::move(converted));
        }
        
        return result;
    }
    
    /**
     * Display a single MNIST image as ASCII art
     * @param image Single MNIST image (784 values)
     * @param width Image width (default 28)
     * @param height Image height (default 28)
     */
    void displayImage(const std::vector<float>& image, int width = 28, int height = 28);
}
