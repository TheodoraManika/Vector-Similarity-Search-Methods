#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cstdint>

/**
 * MNIST Data Parser
 * 
 * Handles parsing of MNIST dataset from binary files.
 * MNIST contains 28x28 grayscale images (784 dimensions) of handwritten digits.
 */
namespace MNIST {
    
    // MNIST format constants
    static constexpr uint32_t MNIST_IMAGE_MAGIC = 2051;
    static constexpr uint32_t MNIST_IMAGE_WIDTH = 28;
    static constexpr uint32_t MNIST_IMAGE_HEIGHT = 28;
    static constexpr uint32_t MNIST_IMAGE_SIZE = MNIST_IMAGE_WIDTH * MNIST_IMAGE_HEIGHT;
    
    /**
     * Parse MNIST images from binary file
     * @param filename Path to MNIST images binary file
     * @param max_images Maximum number of images to load (0 = load all)
     * @param validate_format Whether to perform strict format validation
     * @return Vector of image vectors (each image is flattened to 784 values)
     * @throws std::runtime_error if file cannot be opened or has invalid format
     */
    std::vector<std::vector<float>> parseMNISTImages(const std::string& filename, 
                                                    uint32_t max_images = 0, 
                                                    bool validate_format = true);
}
