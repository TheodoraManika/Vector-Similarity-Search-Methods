#pragma once

#include <random>
#include <vector>

/**
 * Random number generation utilities
 */
namespace Random {
    
    /**
     * Initialize random seed
     */
    void initializeSeed(unsigned int seed = 0);
    
    /**
     * Generate random integer in range [min, max]
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Random integer in range
     */
    int randomInt(int min, int max);
    
    /**
     * Generate random float in range [min, max]
     * @param min Minimum value
     * @param max Maximum value
     * @return Random float in range
     */
    float randomFloat(float min, float max);
    
    /**
     * Generate random double in range [min, max]
     * @param min Minimum value
     * @param max Maximum value
     * @return Random double in range
     */
    double randomDouble(double min, double max);
    
    /**
     * Generate random boolean with given probability
     * @param probability Probability of returning true (0.0 to 1.0)
     * @return Random boolean
     */
    bool randomBool(double probability = 0.5);
    
    /**
     * Generate vector of random integers
     * @param size Size of vector
     * @param min Minimum value
     * @param max Maximum value
     * @return Vector of random integers
     */
    std::vector<int> randomIntVector(size_t size, int min, int max);
    
    /**
     * Generate vector of random floats
     * @param size Size of vector
     * @param min Minimum value
     * @param max Maximum value
     * @return Vector of random floats
     */
    std::vector<float> randomFloatVector(size_t size, float min, float max);
    
    /**
     * Generate vector of random doubles
     * @param size Size of vector
     * @param min Minimum value
     * @param max Maximum value
     * @return Vector of random doubles
     */
    std::vector<double> randomDoubleVector(size_t size, double min, double max);
    
    /**
     * Shuffle a vector randomly
     * @param vec Vector to shuffle
     */
    template<typename T>
    void shuffle(std::vector<T>& vec);
    
    /**
     * Sample n random elements from a vector
     * @param vec Source vector
     * @param n Number of elements to sample
     * @return Vector with sampled elements
     */
    template<typename T>
    std::vector<T> sample(const std::vector<T>& vec, size_t n);
}
