#include "Gaussian.h"
#include <cmath>
#include <chrono>
#include <stdexcept>

namespace Gaussian {
    
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
    
    double generate(double mean, double stddev) {
        if (!seeded) {
            initializeSeed();
        }
        
        std::normal_distribution<double> distribution(mean, stddev);
        return distribution(generator);
    }
    
    std::vector<double> generateVector(size_t size, double mean, double stddev) {
        if (!seeded) {
            initializeSeed();
        }
        
        std::vector<double> result;
        result.reserve(size);
        
        std::normal_distribution<double> distribution(mean, stddev);
        
        for (size_t i = 0; i < size; ++i) {
            result.push_back(distribution(generator));
        }
        
        return result;
    }
    
    double standardNormal() {
        return generate(0.0, 1.0);
    }
    
    double pdf(double x, double mean, double stddev) {
        if (stddev <= 0) {
            throw std::invalid_argument("Standard deviation must be positive");
        }
        
        double exponent = -0.5 * std::pow((x - mean) / stddev, 2);
        double coefficient = 1.0 / (stddev * std::sqrt(2.0 * M_PI));
        
        return coefficient * std::exp(exponent);
    }
}
