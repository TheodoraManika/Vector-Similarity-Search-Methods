#include "MNIST.h"
#include "../../utils/helpers/Utils.h"
#include "../../utils/math/math.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <stdexcept>
#include <random>
#include <chrono>
#include <cmath>

// Fix for potential macro conflicts
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

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
        
        std::cout << "Loaded " << dataset.size() << " MNIST images (" 
                  << rows << "x" << cols << ") from " << filename << std::endl;
        
        return dataset;
    }
    
    std::vector<std::vector<double>> parseMNISTImagesDouble(const std::string& filename, uint32_t max_images) {
        auto float_images = parseMNISTImages(filename, max_images);
        return convertPrecision<float, double>(float_images);
    }
    
    std::vector<int> parseMNISTLabels(const std::string& filename, uint32_t max_labels) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open MNIST labels file: " + filename);
        }
        
        // Read magic number
        uint32_t magic_number;
        file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
        if (file.gcount() != sizeof(magic_number)) {
            throw std::runtime_error("Error reading magic number from " + filename);
        }
        magic_number = our_math::big_to_little_endian(magic_number);
        
        // Validate magic number for labels (2049)
        if (magic_number != MNIST_LABEL_MAGIC) {
            throw std::runtime_error("Invalid MNIST labels file magic number: " + std::to_string(magic_number));
        }
        
        // Read number of labels
        uint32_t n_labels;
        file.read(reinterpret_cast<char*>(&n_labels), sizeof(n_labels));
        if (file.gcount() != sizeof(n_labels)) {
            throw std::runtime_error("Error reading number of labels from " + filename);
        }
        n_labels = our_math::big_to_little_endian(n_labels);
        
        // Apply max_labels limit
        if (max_labels > 0 && max_labels < n_labels) {
            n_labels = max_labels;
        }
        
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
    
    void writeMNISTImages(const std::vector<std::vector<float>>& images, 
                         const std::string& filename, 
                         bool compress) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot create MNIST file: " + filename);
        }
        
        // Write magic number (2051 for images)
        uint32_t magic_number = our_math::big_to_little_endian(MNIST_IMAGE_MAGIC);
        file.write(reinterpret_cast<const char*>(&magic_number), sizeof(magic_number));
        
        // Write number of images
        uint32_t n_images = our_math::big_to_little_endian(static_cast<uint32_t>(images.size()));
        file.write(reinterpret_cast<const char*>(&n_images), sizeof(n_images));
        
        // Write dimensions (assuming 28x28)
        uint32_t rows = our_math::big_to_little_endian(MNIST_IMAGE_HEIGHT);
        uint32_t cols = our_math::big_to_little_endian(MNIST_IMAGE_WIDTH);
        file.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
        file.write(reinterpret_cast<const char*>(&cols), sizeof(cols));
        
        // Write pixel data
        for (const auto& image : images) {
            for (float pixel : image) {
                float clamped = (pixel < 0.0f) ? 0.0f : ((pixel > MNIST_PIXEL_MAX) ? MNIST_PIXEL_MAX : pixel);
                unsigned char pixel_byte = static_cast<unsigned char>(clamped);
                file.write(reinterpret_cast<const char*>(&pixel_byte), sizeof(pixel_byte));
            }
        }
        
        if (compress) {
            std::cout << "Note: Compression not implemented, written uncompressed." << std::endl;
        }
        
        std::cout << "Written " << images.size() << " MNIST images to " << filename << std::endl;
    }
    
    bool validateMNISTDataset(const std::vector<std::vector<float>>& dataset) {
        if (dataset.empty()) {
            std::cerr << "Error: Empty MNIST dataset" << std::endl;
            return false;
        }
        
        // MNIST images should be 28x28 = 784 pixels
        size_t expected_dim = MNIST_IMAGE_SIZE;
        
        for (size_t i = 0; i < dataset.size(); ++i) {
            if (dataset[i].size() != expected_dim) {
                std::cerr << "Error: MNIST image " << i << " has dimension " << dataset[i].size() 
                          << ", expected " << expected_dim << std::endl;
                return false;
            }
            
            // Check pixel values are in valid range [0, 255]
            for (size_t j = 0; j < dataset[i].size(); ++j) {
                if (dataset[i][j] < 0 || dataset[i][j] > MNIST_PIXEL_MAX) {
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
        std::cout << "Image size: " << MNIST_IMAGE_WIDTH << "x" << MNIST_IMAGE_HEIGHT << " pixels" << std::endl;
        
        // Calculate pixel statistics
        float min_pixel = MNIST_PIXEL_MAX;
        float max_pixel = 0.0f;
        double sum = 0.0;
        size_t total_pixels = 0;
        
        for (const auto& image : dataset) {
            for (float pixel : image) {
                if (pixel < min_pixel) min_pixel = pixel;
                if (pixel > max_pixel) max_pixel = pixel;
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
    
    // New functions to match the updated interface
    
    std::string getErrorMessage(MNISTError error) {
        switch (error) {
            case MNISTError::SUCCESS:
                return "Success";
            case MNISTError::FILE_NOT_FOUND:
                return "File not found or cannot be opened";
            case MNISTError::INVALID_MAGIC_NUMBER:
                return "Invalid MNIST magic number";
            case MNISTError::INVALID_DIMENSIONS:
                return "Invalid MNIST dimensions";
            case MNISTError::READ_ERROR:
                return "Error reading file data";
            case MNISTError::MEMORY_ERROR:
                return "Insufficient memory";
            case MNISTError::VALIDATION_ERROR:
                return "Data validation failed";
            default:
                return "Unknown error";
        }
    }
    
    std::pair<std::vector<std::vector<float>>, MNISTError> 
    parseMNISTImagesWithConfig(const std::string& filename, const ParseConfig& config) {
        try {
            auto images = parseMNISTImages(filename, config.max_images, config.validate_format);
            
            // Apply normalization if requested
            if (config.normalize_pixels) {
                images = normalizeMNIST(images);
            }
            
            // Validate if requested
            if (config.validate_format && !validateMNISTDataset(images)) {
                return {std::vector<std::vector<float>>(), MNISTError::VALIDATION_ERROR};
            }
            
            return {std::move(images), MNISTError::SUCCESS};
            
        } catch (const std::invalid_argument&) {
            return {std::vector<std::vector<float>>(), MNISTError::FILE_NOT_FOUND};
        } catch (const std::runtime_error& e) {
            std::string msg = e.what();
            if (msg.find("magic number") != std::string::npos) {
                return {std::vector<std::vector<float>>(), MNISTError::INVALID_MAGIC_NUMBER};
            } else if (msg.find("dimension") != std::string::npos) {
                return {std::vector<std::vector<float>>(), MNISTError::INVALID_DIMENSIONS};
            } else {
                return {std::vector<std::vector<float>>(), MNISTError::READ_ERROR};
            }
        }
    }
    
    void writeMNISTLabels(const std::vector<int>& labels, const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot create MNIST labels file: " + filename);
        }
        
        // Write magic number (2049 for labels)  
        uint32_t magic_number = our_math::big_to_little_endian(MNIST_LABEL_MAGIC);
        file.write(reinterpret_cast<const char*>(&magic_number), sizeof(magic_number));
        
        // Write number of labels
        uint32_t n_labels = our_math::big_to_little_endian(static_cast<uint32_t>(labels.size()));
        file.write(reinterpret_cast<const char*>(&n_labels), sizeof(n_labels));
        
        // Write label data
        for (int label : labels) {
            int clamped_label = (label < 0) ? 0 : ((label > 9) ? 9 : label);
            unsigned char label_byte = static_cast<unsigned char>(clamped_label);
            file.write(reinterpret_cast<const char*>(&label_byte), sizeof(label_byte));
        }
        
        std::cout << "Written " << labels.size() << " MNIST labels to " << filename << std::endl;
    }
    
    // Add simplified versions of the advanced functions for compatibility
    std::vector<std::vector<float>> extractSubset(const std::vector<std::vector<float>>& dataset, 
                                                 size_t start_idx, size_t count) {
        if (start_idx >= dataset.size()) {
            return std::vector<std::vector<float>>();
        }
        
        size_t end_idx = (start_idx + count < dataset.size()) ? start_idx + count : dataset.size();
        std::vector<std::vector<float>> subset;
        subset.reserve(end_idx - start_idx);
        
        for (size_t i = start_idx; i < end_idx; ++i) {
            subset.push_back(dataset[i]);
        }
        
        return subset;
    }
    
    DatasetStats computeDetailedStatistics(const std::vector<std::vector<float>>& dataset) {
        DatasetStats stats;
        
        if (dataset.empty()) {
            return stats;
        }
        
        stats.num_images = dataset.size();
        stats.dimensions = dataset[0].size();
        stats.min_value = MNIST_PIXEL_MAX;
        stats.max_value = 0.0f;
        
        double total_sum = 0.0;
        size_t total_pixels = 0;
        
        // Calculate means
        for (const auto& image : dataset) {
            for (float pixel : image) {
                if (pixel < stats.min_value) stats.min_value = pixel;
                if (pixel > stats.max_value) stats.max_value = pixel;
                total_sum += pixel;
                total_pixels++;
            }
        }
        
        stats.mean_value = static_cast<float>(total_sum / total_pixels);
        
        // Calculate standard deviation
        double variance_sum = 0.0;
        for (const auto& image : dataset) {
            for (float pixel : image) {
                double diff = pixel - stats.mean_value;
                variance_sum += diff * diff;
            }
        }
        
        stats.std_deviation = static_cast<float>(std::sqrt(variance_sum / total_pixels));
        
        return stats;
    }
    
    bool compareDatasets(const std::vector<std::vector<float>>& dataset1,
                        const std::vector<std::vector<float>>& dataset2) {
        if (dataset1.size() != dataset2.size()) {
            std::cout << "Different number of images: " << dataset1.size() 
                      << " vs " << dataset2.size() << std::endl;
            return false;
        }
        
        if (dataset1.empty()) {
            return true;
        }
        
        if (dataset1[0].size() != dataset2[0].size()) {
            std::cout << "Different image dimensions: " << dataset1[0].size() 
                      << " vs " << dataset2[0].size() << std::endl;
            return false;
        }
        
        return true;
    }
    
    // Backward compatibility functions (old signatures)
    std::vector<std::vector<float>> parseMNISTImages(const std::string& filename) {
        return parseMNISTImages(filename, 0, true);
    }
    
    std::vector<std::vector<double>> parseMNISTImagesDouble(const std::string& filename) {
        return parseMNISTImagesDouble(filename, 0);
    }
    
    std::vector<int> parseMNISTLabels(const std::string& filename) {
        return parseMNISTLabels(filename, 0);
    }
    
    void writeMNISTImages(const std::vector<std::vector<float>>& images, const std::string& filename) {
        writeMNISTImages(images, filename, false);
    }
    
}