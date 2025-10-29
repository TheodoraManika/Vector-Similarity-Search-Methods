#include "MNIST.h"
#include "../../utils/math/math.h"
#include <iostream>

namespace MNIST {
    
    std::vector<std::vector<float>> parseMNISTImages(const std::string& filename, 
                                                    uint32_t max_images, 
                                                    bool validate_format) {
        if (filename.empty()) {
            throw std::invalid_argument("Filename cannot be empty");
        }
        
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open MNIST file: " + filename);
        }
        
        // Read magic number
        uint32_t magic_number;
        file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
        if (file.gcount() != sizeof(magic_number)) {
            throw std::runtime_error("Error reading magic number from " + filename);
        }
        magic_number = our_math::big_to_little_endian(magic_number);
        
        // Validate magic number for images (2051)
        if (validate_format && magic_number != MNIST_IMAGE_MAGIC) {
            throw std::runtime_error("Invalid MNIST image file magic number: " + std::to_string(magic_number));
        }
        
        // Read number of images
        uint32_t n_images;
        file.read(reinterpret_cast<char*>(&n_images), sizeof(n_images));
        if (file.gcount() != sizeof(n_images)) {
            throw std::runtime_error("Error reading number of images from " + filename);
        }
        n_images = our_math::big_to_little_endian(n_images);
        
        // Read dimensions
        uint32_t rows, cols;
        file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
        if (file.gcount() != sizeof(rows)) {
            throw std::runtime_error("Error reading rows from " + filename);
        }
        rows = our_math::big_to_little_endian(rows);
        
        file.read(reinterpret_cast<char*>(&cols), sizeof(cols));
        if (file.gcount() != sizeof(cols)) {
            throw std::runtime_error("Error reading columns from " + filename);
        }
        cols = our_math::big_to_little_endian(cols);
        
        if (validate_format && (rows != MNIST_IMAGE_HEIGHT || cols != MNIST_IMAGE_WIDTH)) {
            std::cerr << "Warning: Non-standard MNIST dimensions: " << rows << "x" << cols << std::endl;
        }
        
        // Apply max_images limit
        if (max_images > 0 && max_images < n_images) {
            n_images = max_images;
        }
        
        std::vector<std::vector<float>> dataset;
        dataset.reserve(n_images);
        
        // Read pixel data
        for (uint32_t i = 0; i < n_images; ++i) {
            std::vector<float> image;
            image.reserve(rows * cols);
            
            for (uint32_t j = 0; j < rows * cols; ++j) {
                unsigned char pixel;
                file.read(reinterpret_cast<char*>(&pixel), sizeof(pixel));
                image.push_back(static_cast<float>(pixel));
            }
            
            dataset.push_back(std::move(image));
        }
        
        std::cout << "Loaded " << dataset.size() << " MNIST images of dimension " 
                  << dataset[0].size() << " from " << filename << std::endl;
        
        return dataset;
    }
}
