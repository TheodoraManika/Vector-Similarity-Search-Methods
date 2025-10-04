#include "Random.h"
#include <chrono>
#include <algorithm>
#include <stdexcept>

namespace Random {
    
    static std::mt19937 generator;
    static bool seeded = false;
    
    void initializeSeed(unsigned int seed) {
        if (seed == 0) {
            generator.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        } else {
            generator.seed(seed);
        }
        seeded = true;
    }
    
    int randomInt(int min, int max) {
        if (!seeded) {
            initializeSeed();
        }
        
        std::uniform_int_distribution<int> distribution(min, max);
        return distribution(generator);
    }
    
    float randomFloat(float min, float max) {
        if (!seeded) {
            initializeSeed();
        }
        
        std::uniform_real_distribution<float> distribution(min, max);
        return distribution(generator);
    }
    
    double randomDouble(double min, double max) {
        if (!seeded) {
            initializeSeed();
        }
        
        std::uniform_real_distribution<double> distribution(min, max);
        return distribution(generator);
    }
    
    bool randomBool(double probability) {
        if (!seeded) {
            initializeSeed();
        }
        
        std::uniform_real_distribution<double> distribution(0.0, 1.0);
        return distribution(generator) < probability;
    }
    
    std::vector<int> randomIntVector(size_t size, int min, int max) {
        std::vector<int> result;
        result.reserve(size);
        
        for (size_t i = 0; i < size; ++i) {
            result.push_back(randomInt(min, max));
        }
        
        return result;
    }
    
    std::vector<float> randomFloatVector(size_t size, float min, float max) {
        std::vector<float> result;
        result.reserve(size);
        
        for (size_t i = 0; i < size; ++i) {
            result.push_back(randomFloat(min, max));
        }
        
        return result;
    }
    
    std::vector<double> randomDoubleVector(size_t size, double min, double max) {
        std::vector<double> result;
        result.reserve(size);
        
        for (size_t i = 0; i < size; ++i) {
            result.push_back(randomDouble(min, max));
        }
        
        return result;
    }
    
    template<typename T>
    void shuffle(std::vector<T>& vec) {
        if (!seeded) {
            initializeSeed();
        }
        
        std::shuffle(vec.begin(), vec.end(), generator);
    }
    
    template<typename T>
    std::vector<T> sample(const std::vector<T>& vec, size_t n) {
        if (n > vec.size()) {
            throw std::invalid_argument("Sample size cannot be larger than vector size");
        }
        
        std::vector<T> result = vec;
        shuffle(result);
        result.resize(n);
        return result;
    }
    
    // Explicit template instantiations
    template void shuffle<int>(std::vector<int>&);
    template void shuffle<float>(std::vector<float>&);
    template void shuffle<double>(std::vector<double>&);
    
    template std::vector<int> sample<int>(const std::vector<int>&, size_t);
    template std::vector<float> sample<float>(const std::vector<float>&, size_t);
    template std::vector<double> sample<double>(const std::vector<double>&, size_t);
}
