#include "MNIST.h"
#include "../../utils/helpers/Utils.h"
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
        magic_number = Utils::Convert::bigToLittleEndian(magic_number);
        
        if (magic_number != 0x00000803) {
            throw std::runtime_error("Invalid MNIST magic number: " + std::to_string(magic_number));
        }
        
        // Read number of images
        uint32_t n_images;
        file.read(reinterpret_cast<char*>(&n_images), sizeof(n_images));
        n_images = Utils::Convert::bigToLittleEndian(n_images);
        
        // Read dimensions
        uint32_t rows, cols;
        file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
        rows = Utils::Convert::bigToLittleEndian(rows);
        file.read(reinterpret_cast<char*>(&cols), sizeof(cols));
        cols = Utils::Convert::bigToLittleEndian(cols);
        
        if (rows != 28 || cols != 28) {
            std::cerr << "Warning: Expected 28x28 images, got " << rows << "x" << cols << std::endl;
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
        
        std::cout << "Loaded " << dataset.size() << " MNIST images (" 
                  << rows << "x" << cols << ") from " << filename << std::endl;
        
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
        magic_number = Utils::Convert::bigToLittleEndian(magic_number);
        
        if (magic_number != 0x00000801) {
            throw std::runtime_error("Invalid MNIST labels magic number: " + std::to_string(magic_number));
        }
        
        // Read number of labels
        uint32_t n_labels;
        file.read(reinterpret_cast<char*>(&n_labels), sizeof(n_labels));
        n_labels = Utils::Convert::bigToLittleEndian(n_labels);
        
        std::vector<int> labels;
        labels.reserve(n_labels);
        
        // Read labels
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
        
        // Write magic number
        uint32_t magic_number = Utils::Convert::bigToLittleEndian(0x00000803);
        file.write(reinterpret_cast<const char*>(&magic_number), sizeof(magic_number));
        
        // Write number of images
        uint32_t n_images = Utils::Convert::bigToLittleEndian(static_cast<uint32_t>(images.size()));
        file.write(reinterpret_cast<const char*>(&n_images), sizeof(n_images));
        
        // Write dimensions (assuming 28x28)
        uint32_t rows = Utils::Convert::bigToLittleEndian(28);
        uint32_t cols = Utils::Convert::bigToLittleEndian(28);
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
        
        // Check if all images have 784 dimensions (28x28)
        for (size_t i = 0; i < dataset.size(); ++i) {
            if (dataset[i].size() != 784) {
                std::cerr << "Error: Image " << i << " has " << dataset[i].size() 
                          << " dimensions, expected 784" << std::endl;
                return false;
            }
            
            // Check for valid pixel values (0-255)
            for (size_t j = 0; j < dataset[i].size(); ++j) {
                if (dataset[i][j] < 0 || dataset[i][j] > 255) {
                    std::cerr << "Warning: Pixel value " << dataset[i][j] 
                              << " out of range [0, 255] in image " << i << std::endl;
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
        std::cout << "Image dimension: " << dataset[0].size() << " (28x28)" << std::endl;
        
        // Calculate min, max, mean pixel values
        float min_val = 255.0f;
        float max_val = 0.0f;
        double sum = 0.0;
        size_t total_pixels = 0;
        
        for (const auto& image : dataset) {
            for (float pixel : image) {
                min_val = std::min(min_val, pixel);
                max_val = std::max(max_val, pixel);
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
        
        std::cout << "Pixel value range: [" << min_val << ", " << max_val << "]" << std::endl;
        std::cout << "Mean pixel value: " << mean << std::endl;
        std::cout << "Standard deviation: " << stddev << std::endl;
        
        std::cout << "==============================" << std::endl;
    }
    
    std::vector<std::vector<float>> normalizeMNIST(const std::vector<std::vector<float>>& images) {
        std::vector<std::vector<float>> normalized;
        normalized.reserve(images.size());
        
        for (const auto& image : images) {
            std::vector<float> norm_image;
            norm_image.reserve(image.size());
            
            // Normalize pixel values from [0, 255] to [0, 1]
            for (float pixel : image) {
                norm_image.push_back(pixel / 255.0f);
            }
            
            normalized.push_back(std::move(norm_image));
        }
        
        return normalized;
    }
    
    void displayImage(const std::vector<float>& image, int width, int height) {
        if (image.size() != static_cast<size_t>(width * height)) {
            std::cerr << "Error: Image size mismatch" << std::endl;
            return;
        }
        
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                float pixel = image[row * width + col];
                
                // Convert to ASCII representation
                if (pixel < 64) {
                    std::cout << " ";
                } else if (pixel < 128) {
                    std::cout << ".";
                } else if (pixel < 192) {
                    std::cout << "o";
                } else {
                    std::cout << "#";
                }
            }
            std::cout << std::endl;
        }
    }
    
    // Template specializations
    template<>
    std::vector<std::vector<double>> convertPrecision<float, double>(const std::vector<std::vector<float>>& images) {
        std::vector<std::vector<double>> result;
        result.reserve(images.size());
        
        for (const auto& img : images) {
            std::vector<double> converted;
            converted.reserve(img.size());
            
            for (float val : img) {
                converted.push_back(static_cast<double>(val));
            }
            
            result.push_back(std::move(converted));
        }
        
        return result;
    }
    
    template<>
    std::vector<std::vector<float>> convertPrecision<double, float>(const std::vector<std::vector<double>>& images) {
        std::vector<std::vector<float>> result;
        result.reserve(images.size());
        
        for (const auto& img : images) {
            std::vector<float> converted;
            converted.reserve(img.size());
            
            for (double val : img) {
                converted.push_back(static_cast<float>(val));
            }
            
            result.push_back(std::move(converted));
        }
        
        return result;
    }
}