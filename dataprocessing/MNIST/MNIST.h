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
    
    // MNIST format constants
    static constexpr uint32_t MNIST_IMAGE_MAGIC = 2051;
    static constexpr uint32_t MNIST_LABEL_MAGIC = 2049;
    static constexpr uint32_t MNIST_IMAGE_WIDTH = 28;
    static constexpr uint32_t MNIST_IMAGE_HEIGHT = 28;
    static constexpr uint32_t MNIST_IMAGE_SIZE = MNIST_IMAGE_WIDTH * MNIST_IMAGE_HEIGHT;
    static constexpr uint32_t MNIST_NUM_CLASSES = 10;
    static constexpr float MNIST_PIXEL_MAX = 255.0f;
    
    // Configuration structure for parsing
    struct ParseConfig {
        uint32_t max_images = 0;           // 0 = load all
        bool validate_format = true;       // Strict format validation
        bool normalize_pixels = false;     // Auto-normalize to [0,1]
        bool verbose_output = true;        // Print loading progress
        size_t memory_limit_mb = 0;        // 0 = no limit
    };
    
    // Error codes for better error handling
    enum class MNISTError {
        SUCCESS = 0,
        FILE_NOT_FOUND,
        INVALID_MAGIC_NUMBER,
        INVALID_DIMENSIONS,
        READ_ERROR,
        MEMORY_ERROR,
        VALIDATION_ERROR
    };
    
    /**
     * Get error message for MNIST error code
     * @param error Error code
     * @return Human-readable error message
     */
    std::string getErrorMessage(MNISTError error);
    
    /**
     * Parse MNIST images from binary file
     * @param filename Path to MNIST images binary file
     * @param max_images Maximum number of images to load (0 = load all)
     * @param validate_format Whether to perform strict format validation
     * @return Vector of image vectors (each image is flattened to 784 values)
     * @throws std::runtime_error if file cannot be opened or has invalid format
     * @throws std::invalid_argument if filename is empty
     */
    std::vector<std::vector<float>> parseMNISTImages(const std::string& filename, 
                                                    uint32_t max_images = 0, 
                                                    bool validate_format = true);
    
    /**
     * Parse MNIST images with configuration
     * @param filename Path to MNIST images binary file
     * @param config Parsing configuration
     * @return Vector of image vectors and error status
     */
    std::pair<std::vector<std::vector<float>>, MNISTError> 
    parseMNISTImagesWithConfig(const std::string& filename, const ParseConfig& config = ParseConfig{});
    
    /**
     * Parse MNIST images from binary file (double precision)
     * @param filename Path to MNIST images binary file
     * @param max_images Maximum number of images to load (0 = load all)
     * @return Vector of image vectors in double precision
     */
    std::vector<std::vector<double>> parseMNISTImagesDouble(const std::string& filename, uint32_t max_images = 0);
    
    /**
     * Parse MNIST labels from binary file
     * @param filename Path to MNIST labels binary file
     * @param max_labels Maximum number of labels to load (0 = load all)
     * @return Vector of labels (0-9)
     * @throws std::runtime_error if file cannot be opened or has invalid format
     */
    std::vector<int> parseMNISTLabels(const std::string& filename, uint32_t max_labels = 0);
    
    /**
     * Write MNIST images to binary file
     * @param images Image vectors to write
     * @param filename Output file path
     * @param compress Whether to apply compression
     */
    void writeMNISTImages(const std::vector<std::vector<float>>& images, 
                         const std::string& filename, 
                         bool compress = false);
    
    /**
     * Write MNIST labels to binary file
     * @param labels Label vector to write
     * @param filename Output file path
     */
    void writeMNISTLabels(const std::vector<int>& labels, const std::string& filename);
    
    /**
     * Estimate memory requirements for dataset
     * @param filename MNIST file path
     * @return Estimated memory in bytes
     */
    size_t estimateMemoryRequirement(const std::string& filename);
    
    /**
     * Check if system has enough memory for dataset
     * @param filename MNIST file path
     * @return True if sufficient memory available
     */
    bool checkMemoryAvailability(const std::string& filename);
    
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
    
    // Advanced processing functions
    
    /**
     * Extract a subset of MNIST dataset
     * @param dataset Full MNIST dataset
     * @param start_idx Starting index
     * @param count Number of images to extract
     * @return Subset of the dataset
     */
    std::vector<std::vector<float>> extractSubset(const std::vector<std::vector<float>>& dataset, 
                                                 size_t start_idx, size_t count);
    
    /**
     * Filter MNIST dataset by specific labels
     * @param images MNIST images
     * @param labels Corresponding labels
     * @param target_labels Labels to keep
     * @return Filtered images and labels
     */
    std::pair<std::vector<std::vector<float>>, std::vector<int>> 
    filterByLabels(const std::vector<std::vector<float>>& images, 
                   const std::vector<int>& labels, 
                   const std::vector<int>& target_labels);
    
    /**
     * Apply data augmentation to MNIST images
     * @param images Original images
     * @param noise_level Gaussian noise level (0.0 - 1.0)
     * @param shift_pixels Maximum pixel shift for translation
     * @return Augmented dataset
     */
    std::vector<std::vector<float>> augmentData(const std::vector<std::vector<float>>& images,
                                               float noise_level = 0.1f,
                                               int shift_pixels = 2);
    
    /**
     * Compute dataset statistics for analysis
     * @param dataset MNIST dataset
     * @return Statistics structure
     */
    struct DatasetStats {
        size_t num_images;
        size_t dimensions;
        float min_value, max_value;
        float mean_value, std_deviation;
        std::vector<float> per_pixel_mean;
        std::vector<float> per_pixel_std;
    };
    
    DatasetStats computeDetailedStatistics(const std::vector<std::vector<float>>& dataset);
    
    /**
     * Memory-efficient streaming parser for large datasets
     * @param filename MNIST file path
     * @param batch_size Number of images to load per batch
     * @param callback Function to process each batch
     */
    template<typename Callback>
    void streamingParse(const std::string& filename, size_t batch_size, Callback callback) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open MNIST file: " + filename);
        }
        
        // Read header
        uint32_t magic_number, n_images, rows, cols;
        file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
        magic_number = our_math::big_to_little_endian(magic_number);
        
        if (magic_number != 2051) {
            throw std::runtime_error("Invalid MNIST image file magic number");
        }
        
        file.read(reinterpret_cast<char*>(&n_images), sizeof(n_images));
        n_images = our_math::big_to_little_endian(n_images);
        file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
        rows = our_math::big_to_little_endian(rows);
        file.read(reinterpret_cast<char*>(&cols), sizeof(cols));
        cols = our_math::big_to_little_endian(cols);
        
        // Process in batches
        size_t images_per_batch = std::min(batch_size, static_cast<size_t>(n_images));
        size_t pixel_count = rows * cols;
        
        for (uint32_t start = 0; start < n_images; start += images_per_batch) {
            size_t actual_batch_size = std::min(images_per_batch, static_cast<size_t>(n_images - start));
            
            std::vector<std::vector<float>> batch;
            batch.reserve(actual_batch_size);
            
            for (size_t i = 0; i < actual_batch_size; ++i) {
                std::vector<float> image;
                image.reserve(pixel_count);
                
                for (size_t j = 0; j < pixel_count; ++j) {
                    unsigned char pixel;
                    file.read(reinterpret_cast<char*>(&pixel), sizeof(pixel));
                    image.push_back(static_cast<float>(pixel));
                }
                
                batch.push_back(std::move(image));
            }
            
            // Call user callback with this batch
            callback(batch, start);
        }
    }
    
    /**
     * Save dataset statistics to file
     * @param stats Dataset statistics
     * @param filename Output file path
     */
    void saveStatistics(const DatasetStats& stats, const std::string& filename);
    
    /**
     * Compare two MNIST datasets for consistency
     * @param dataset1 First dataset
     * @param dataset2 Second dataset
     * @return True if datasets are compatible
     */
    bool compareDatasets(const std::vector<std::vector<float>>& dataset1,
                        const std::vector<std::vector<float>>& dataset2);
}
