#include "metrics.h"

namespace metrics {
    
    template <typename T>
    T ManhattanDistance(std::vector<T> x, std::vector<T> y, int dim) {
        if (static_cast<int>(x.size()) < dim || static_cast<int>(y.size()) < dim) {
            throw std::invalid_argument("Vector dimensions are smaller than specified dimension");
        }
        
        T distance = 0;
        for (int i = 0; i < dim; ++i) {
            distance += std::abs(x[i] - y[i]);
        }
        return distance;
    }
    
    template <typename T>
    T EuclideanDistance(std::vector<T> x, std::vector<T> y, int dim) {
        if (static_cast<int>(x.size()) < dim || static_cast<int>(y.size()) < dim) {
            throw std::invalid_argument("Vector dimensions are smaller than specified dimension");
        }
        
        T distance_squared = 0;
        for (int i = 0; i < dim; ++i) {
            T diff = x[i] - y[i];
            distance_squared += diff * diff;
        }
        return std::sqrt(distance_squared);
    }
    
    template <typename T>
    T EuclideanDistanceSquared(std::vector<T> x, std::vector<T> y, int dim) {
        if (static_cast<int>(x.size()) < dim || static_cast<int>(y.size()) < dim) {
            throw std::invalid_argument("Vector dimensions are smaller than specified dimension");
        }
        
        T distance_squared = 0;
        for (int i = 0; i < dim; ++i) {
            T diff = x[i] - y[i];
            distance_squared += diff * diff;
        }
        return distance_squared;
    }
    
    // Explicit template instantiations
    template float ManhattanDistance<float>(std::vector<float> x, std::vector<float> y, int dim);
    template double ManhattanDistance<double>(std::vector<double> x, std::vector<double> y, int dim);
    template int ManhattanDistance<int>(std::vector<int> x, std::vector<int> y, int dim);
    
    template float EuclideanDistance<float>(std::vector<float> x, std::vector<float> y, int dim);
    template double EuclideanDistance<double>(std::vector<double> x, std::vector<double> y, int dim);
    template int EuclideanDistance<int>(std::vector<int> x, std::vector<int> y, int dim);
    
    template float EuclideanDistanceSquared<float>(std::vector<float> x, std::vector<float> y, int dim);
    template double EuclideanDistanceSquared<double>(std::vector<double> x, std::vector<double> y, int dim);
    template int EuclideanDistanceSquared<int>(std::vector<int> x, std::vector<int> y, int dim);
}
