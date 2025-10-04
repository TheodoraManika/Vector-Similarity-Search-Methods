#pragma once

#include <random>
#include <vector>

/**
 * Gaussian distribution utilities
 */
namespace Gaussian {
    
    /**
     * Generate a random number from Gaussian distribution
     * @param mean Mean of the distribution
     * @param stddev Standard deviation of the distribution
     * @return Random number from Gaussian distribution
     */
    double generate(double mean = 0.0, double stddev = 1.0);
    
    /**
     * Generate a vector of random numbers from Gaussian distribution
     * @param size Size of the vector
     * @param mean Mean of the distribution
     * @param stddev Standard deviation of the distribution
     * @return Vector of random numbers from Gaussian distribution
     */
    std::vector<double> generateVector(size_t size, double mean = 0.0, double stddev = 1.0);
    
    /**
     * Generate a random number from standard normal distribution (mean=0, stddev=1)
     * @return Random number from standard normal distribution
     */
    double standardNormal();
    
    /**
     * Initialize random seed
     */
    void initializeSeed(unsigned int seed = 0);
    
    /**
     * Get the probability density function value at x
     * @param x Value to evaluate
     * @param mean Mean of the distribution
     * @param stddev Standard deviation of the distribution
     * @return PDF value at x
     */
    double pdf(double x, double mean = 0.0, double stddev = 1.0);
}
