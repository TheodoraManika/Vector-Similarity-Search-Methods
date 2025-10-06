#include "MNIST.h"
#include "../../utils/math/math.h"
#include "../../metrics/metrics.h"
#include <iostream>
#include <algorithm>
#include <numeric>

namespace MNIST {
    
    std::vector<std::vector<float>> parseMNISTImages(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open MNIST file: " + filename);
        }
        
        // Read magic number
        uint32_t magic_number;
        file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
        magic_number = our_math::big_to_little_endian(magic_number);
        
        // Validate magic number for images (2051)
        if (magic_number != 2051) {
            throw std::runtime_error("Invalid MNIST image file magic number: " + std::to_string(magic_number));
        }
        
        // Read number of images
        uint32_t n_images;
        file.read(reinterpret_cast<char*>(&n_images), sizeof(n_images));
        n_images = our_math::big_to_little_endian(n_images);
        
        // Read dimensions
        uint32_t rows, cols;
        file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
        rows = our_math::big_to_little_endian(rows);
        file.read(reinterpret_cast<char*>(&cols), sizeof(cols));
        cols = our_math::big_to_little_endian(cols);
        
        if (rows != 28 || cols != 28) {
            std::cerr << "Warning: Non-standard MNIST dimensions: " << rows << "x" << cols << std::endl;
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
    
    std::vector<std::vector<double>> parseMNISTImagesDouble(const std::string& filename) {
        auto float_images = parseMNISTImages(filename);
        return convertPrecision<float, double>(float_images);
    }
    
    std::vector<int> parseMNISTLabels(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open MNIST labels file: " + filename);
        }
        
        // Read magic number
        uint32_t magic_number;
        file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
        magic_number = our_math::big_to_little_endian(magic_number);
        
        // Validate magic number for labels (2049)
        if (magic_number != 2049) {
            throw std::runtime_error("Invalid MNIST labels file magic number: " + std::to_string(magic_number));
        }
        
        // Read number of labels
        uint32_t n_labels;
        file.read(reinterpret_cast<char*>(&n_labels), sizeof(n_labels));
        n_labels = our_math::big_to_little_endian(n_labels);
        
        std::vector<int> labels;
        labels.reserve(n_labels);
        
        // Read label data
        for (uint32_t i = 0; i < n_labels; ++i) {
            unsigned char label;
            file.read(reinterpret_cast<char*>(&label), sizeof(label));
            labels.push_back(static_cast<int>(label));
        }
        
        std::cout << "Loaded " << labels.size() << " MNIST labels from " << filename << std::endl;
        
        return labels;
    }
    
    void writeMNISTImages(const std::vector<std::vector<float>>& images, const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot create MNIST file: " + filename);
        }
        
        // Write magic number (2051 for images)
        uint32_t magic_number = our_math::big_to_little_endian(2051);
        file.write(reinterpret_cast<const char*>(&magic_number), sizeof(magic_number));
        
        // Write number of images
        uint32_t n_images = our_math::big_to_little_endian(static_cast<uint32_t>(images.size()));
        file.write(reinterpret_cast<const char*>(&n_images), sizeof(n_images));
        
        // Write dimensions (assuming 28x28)
        uint32_t rows = our_math::big_to_little_endian(28);
        uint32_t cols = our_math::big_to_little_endian(28);
        file.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
        file.write(reinterpret_cast<const char*>(&cols), sizeof(cols));
        
        // Write pixel data
        for (const auto& image : images) {
            for (float pixel : image) {
                unsigned char pixel_byte = static_cast<unsigned char>(std::max(0.0f, std::min(255.0f, pixel)));
                file.write(reinterpret_cast<const char*>(&pixel_byte), sizeof(pixel_byte));
            }
        }
        
        std::cout << "Written " << images.size() << " MNIST images to " << filename << std::endl;
    }
    
    bool validateMNISTDataset(const std::vector<std::vector<float>>& dataset) {
        if (dataset.empty()) {
            std::cerr << "Error: Empty MNIST dataset" << std::endl;
            return false;
        }
        
        // MNIST images should be 28x28 = 784 pixels
        size_t expected_dim = 784;
        
        for (size_t i = 0; i < dataset.size(); ++i) {
            if (dataset[i].size() != expected_dim) {
                std::cerr << "Error: MNIST image " << i << " has dimension " << dataset[i].size() 
                          << ", expected " << expected_dim << std::endl;
                return false;
            }
            
            // Check pixel values are in valid range [0, 255]
            for (size_t j = 0; j < dataset[i].size(); ++j) {
                if (dataset[i][j] < 0 || dataset[i][j] > 255) {
                    std::cerr << "Error: Invalid pixel value " << dataset[i][j] 
                              << " in image " << i << " at pixel " << j << std::endl;
                    return false;
                }
            }
        }
        
        return true;
    }
    
    void printMNISTStatistics(const std::vector<std::vector<float>>& dataset) {
        if (dataset.empty()) {
            std::cout << "Empty MNIST dataset" << std::endl;
            return;
        }
        
        std::cout << "=== MNIST Dataset Statistics ===" << std::endl;
        std::cout << "Number of images: " << dataset.size() << std::endl;
        std::cout << "Image dimension: " << dataset[0].size() << " pixels" << std::endl;
        std::cout << "Image size: 28x28 pixels" << std::endl;
        
        // Calculate pixel statistics
        float min_pixel = 255.0f;
        float max_pixel = 0.0f;
        double sum = 0.0;
        size_t total_pixels = 0;
        
        for (const auto& image : dataset) {
            for (float pixel : image) {
                min_pixel = std::min(min_pixel, pixel);
                max_pixel = std::max(max_pixel, pixel);
                sum += pixel;
                total_pixels++;
            }
        }
        
        double mean = sum / total_pixels;
        
        // Calculate standard deviation
        double variance_sum = 0.0;
        for (const auto& image : dataset) {
            for (float pixel : image) {
                double diff = pixel - mean;
                variance_sum += diff * diff;
            }
        }
        double stddev = std::sqrt(variance_sum / total_pixels);
        
        std::cout << "Pixel value range: [" << min_pixel << ", " << max_pixel << "]" << std::endl;
        std::cout << "Mean pixel value: " << mean << std::endl;
        std::cout << "Pixel standard deviation: " << stddev << std::endl;
        
        std::cout << "================================" << std::endl;
    }
    
    std::vector<std::vector<float>> normalizeMNIST(const std::vector<std::vector<float>>& images) {
        std::vector<std::vector<float>> normalized;
        normalized.reserve(images.size());
        
        for (const auto& image : images) {
            std::vector<float> norm_image;
            norm_image.reserve(image.size());
            
            // Normalize pixels from [0, 255] to [0, 1]
            for (float pixel : image) {
                norm_image.push_back(pixel / 255.0f);
            }
            
            normalized.push_back(std::move(norm_image));
        }
        
        return normalized;
    }
    
    void displayImage(const std::vector<float>& image, int width, int height) {
        if (image.size() != static_cast<size_t>(width * height)) {
            std::cerr << "Error: Image size " << image.size() 
                      << " doesn't match dimensions " << width << "x" << height << std::endl;
            return;
        }
        
        std::cout << "MNIST Image (" << width << "x" << height << "):" << std::endl;
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float pixel = image[y * width + x];
                
                // Convert pixel value to ASCII character
                char ch;
                if (pixel < 64) ch = ' ';
                else if (pixel < 128) ch = '.';
                else if (pixel < 192) ch = '+';
                else ch = '#';
                
                std::cout << ch;
            }
            std::cout << std::endl;
        }
    }
}